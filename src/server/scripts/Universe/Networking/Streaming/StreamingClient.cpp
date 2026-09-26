/*
 * This file is part of the UniverseEmu Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Original module: mod-streamingclient by brian8544 (AzerothCore)
 * https://github.com/brian8544/mod-streamingclient
 * Ported to UniverseEmu conventions (WorldScript hooks, sConfigMgr::Get*Default,
 * SC_LOG_* macros, manual registration via universe_script_loader.cpp instead of
 * AzerothCore's modules/ auto-loader).
 *
 * Embeds a small HTTP file server into worldserver so players can connect with a
 * lightweight "streaming" client (a few MB) that downloads the rest of the data
 * while playing, the way Cataclysm and later clients do. Serves the MPQ files
 * found under DataDir/cdn, with HTTP Range support for resumable/partial reads.
 *
 * Disclaimer (kept from upstream): convenient plug-and-play option for small
 * servers. For production use, prefer a dedicated web server (Nginx/Apache) -
 * this HTTP server has not been hardened for security or high load.
 */

#include "ScriptMgr.h"
#include "Config.h"
#include "Log.h"
#include "CryptoHash.h"

#include <boost/asio.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <memory>
#include <vector>
#include <thread>
#include <atomic>
#include <regex>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;
using boost::asio::ip::tcp;

static constexpr size_t   MAX_HEADER_BYTES = 8192;
static constexpr uintmax_t MAX_FILE_SIZE   = 32ULL * 1024 * 1024 * 1024;
static constexpr size_t   FILE_CHUNK_BYTES = 256 * 1024;
static constexpr int      ACCEPT_BACKLOG   = boost::asio::socket_base::max_listen_connections;
static std::regex const   s_rangeRe(R"(bytes=(\d+)-(\d*))");

static fs::path SafeJoin(fs::path const& root, std::string const& target)
{
    if (target.find('\0') != std::string::npos)
        return {};

    fs::path candidate = (root / fs::path(target)).lexically_normal();

    auto [rootEnd, _] = std::mismatch(root.begin(), root.end(),
                                      candidate.begin(), candidate.end());
    if (rootEnd != root.end())
        return {};

    return candidate;
}

static uint32_t const* Crc32Table()
{
    static uint32_t table[256];
    static bool initialized = false;
    if (!initialized)
    {
        for (uint32_t i = 0; i < 256; ++i)
        {
            uint32_t c = i;
            for (int k = 0; k < 8; ++k)
                c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
            table[i] = c;
        }
        initialized = true;
    }
    return table;
}

struct Crc32Accumulator
{
    uint32_t crc = 0xFFFFFFFFu;

    void Update(uint8 const* data, size_t len)
    {
        uint32_t const* table = Crc32Table();
        for (size_t i = 0; i < len; ++i)
            crc = table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    }

    uint32_t Finalize() const { return crc ^ 0xFFFFFFFFu; }
};

static std::string       g_signatureFileContent;
static std::string       g_wowMfilLines;
static unsigned int      g_wowMfilBuildNumber = 12340;
static std::atomic<bool> g_shuttingDown{ false };

struct HashCacheEntry
{
    uintmax_t   size;
    uintmax_t   mtime;
    std::string md5Hex;
    uint32_t    crc32;
};

static std::unordered_map<std::string, HashCacheEntry> LoadHashCache(fs::path const& cachePath)
{
    std::unordered_map<std::string, HashCacheEntry> cache;

    std::ifstream in(cachePath);
    if (!in.is_open())
        return cache;

    std::string line;
    while (std::getline(in, line))
    {
        size_t p1 = line.find('\t');
        size_t p2 = (p1 == std::string::npos) ? std::string::npos : line.find('\t', p1 + 1);
        size_t p3 = (p2 == std::string::npos) ? std::string::npos : line.find('\t', p2 + 1);
        size_t p4 = (p3 == std::string::npos) ? std::string::npos : line.find('\t', p3 + 1);
        if (p1 == std::string::npos || p2 == std::string::npos || p3 == std::string::npos || p4 == std::string::npos)
            continue;

        HashCacheEntry entry;
        try
        {
            entry.size  = std::stoull(line.substr(p1 + 1, p2 - p1 - 1));
            entry.mtime = std::stoull(line.substr(p2 + 1, p3 - p2 - 1));
            entry.crc32 = static_cast<uint32_t>(std::stoull(line.substr(p3 + 1, p4 - p3 - 1)));
        }
        catch (...)
        {
            continue;
        }
        entry.md5Hex = line.substr(p4 + 1);
        cache[line.substr(0, p1)] = std::move(entry);
    }

    return cache;
}

static void SaveHashCache(fs::path const& cachePath, std::unordered_map<std::string, HashCacheEntry> const& cache)
{
    std::ofstream out(cachePath, std::ios::trunc);
    if (!out.is_open())
    {
        SC_LOG_WARN("custom.streamingclient", "Could not write hash cache '{}' -- every restart will re-hash the full CDN.", cachePath.string());
        return;
    }

    for (auto const& [relPath, entry] : cache)
        out << relPath << '\t' << entry.size << '\t' << entry.mtime << '\t' << entry.crc32 << '\t' << entry.md5Hex << '\n';
}

struct BuiltManifests
{
    std::string signatureFile;
    std::string wowMfilLines;
};

static BuiltManifests BuildSignatureFile(fs::path const& cdnRoot, fs::path const& cachePath, unsigned int buildNumber)
{
    std::ostringstream header;
    std::ostringstream mfilLines;
    size_t fileCount   = 0;
    size_t reusedCount = 0;

    std::unordered_map<std::string, HashCacheEntry> oldCache = LoadHashCache(cachePath);
    std::unordered_map<std::string, HashCacheEntry> newCache;

    std::error_code ec;
    for (auto const& entry : fs::recursive_directory_iterator(cdnRoot, ec))
    {
        if (g_shuttingDown.load())
            break;

        if (ec || !entry.is_regular_file())
            continue;

        fs::path relative = fs::relative(entry.path(), cdnRoot, ec);
        if (ec)
            continue;

        std::string relStr = relative.generic_string();

        std::string scope = "base";
        auto slashPos = relStr.find('/');
        if (slashPos != std::string::npos && slashPos == 4)
            scope = relStr.substr(0, slashPos);

        std::error_code statEc;
        uintmax_t curSize = fs::file_size(entry.path(), statEc);
        if (statEc)
            continue;
        auto curMtime = static_cast<uintmax_t>(fs::last_write_time(entry.path(), statEc).time_since_epoch().count());
        if (statEc)
            continue;

        std::string md5Hex;
        uint32_t    crc32Value = 0;

        auto cacheIt = oldCache.find(relStr);
        bool cacheHit = cacheIt != oldCache.end()
            && cacheIt->second.size == curSize
            && cacheIt->second.mtime == curMtime;

        if (cacheHit)
        {
            md5Hex     = cacheIt->second.md5Hex;
            crc32Value = cacheIt->second.crc32;
            ++reusedCount;
        }
        else
        {
            std::ifstream file(entry.path(), std::ios::binary);
            if (!file.is_open())
                continue;

            SC_LOG_INFO("custom.streamingclient", "Hashing '{}' (new or modified since last cache)...", relStr);

            Syphrena::Crypto::MD5 hash;
            Crc32Accumulator      crc;
            std::vector<char> buffer(1 * 1024 * 1024);
            while (file)
            {
                file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
                std::streamsize got = file.gcount();
                if (got <= 0)
                    break;
                auto* data = reinterpret_cast<uint8 const*>(buffer.data());
                hash.UpdateData(data, static_cast<size_t>(got));
                crc.Update(data, static_cast<size_t>(got));
            }
            hash.Finalize();
            crc32Value = crc.Finalize();

            std::ostringstream hex;
            hex << std::uppercase << std::hex << std::setfill('0');
            for (uint8 b : hash.GetDigest())
                hex << std::setw(2) << static_cast<unsigned int>(b);
            md5Hex = hex.str();
        }

        newCache[relStr] = HashCacheEntry{ curSize, curMtime, md5Hex, crc32Value };
        header << scope << ";c;" << md5Hex << ";Data/" << relStr << "\r\n";
        mfilLines << "Data/" << relStr << ";" << crc32Value << ";" << buildNumber << ";0\n";
        ++fileCount;
    }

    SaveHashCache(cachePath, newCache);

    SC_LOG_INFO("custom.streamingclient",
                "Signature file built ({} file(s), {} reused from cache, under '{}').",
                fileCount, reusedCount, cdnRoot.string());

    std::string result = header.str();
    result += "NGIS";
    result.append(32, '\0');
    return { result, mfilLines.str() };
}

class HttpServer
{
public:
    HttpServer(unsigned short port, std::string const& root, unsigned int threads)
        : io_ctx()
        , acceptor(io_ctx)
        , endpoint(tcp::v4(), port)
        , root_path(fs::weakly_canonical(fs::path(root)))
        , threads_count(threads)
        , running(false)
    {}

    ~HttpServer() { Stop(); }

    void Start()
    {
        if (running.exchange(true))
            return;

        boost::system::error_code ec;

        acceptor.open(endpoint.protocol(), ec);
        if (ec) { SC_LOG_ERROR("custom.streamingclient", "acceptor open: {}", ec.message()); return; }

        acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);

        acceptor.bind(endpoint, ec);
        if (ec) { SC_LOG_ERROR("custom.streamingclient", "acceptor bind: {}", ec.message()); return; }

        acceptor.listen(ACCEPT_BACKLOG, ec);
        if (ec) { SC_LOG_ERROR("custom.streamingclient", "acceptor listen: {}", ec.message()); return; }

        DoAccept();

        for (unsigned i = 0; i < threads_count; ++i)
            threads.emplace_back([this]() { io_ctx.run(); });

        SC_LOG_INFO("custom.streamingclient", "HTTP server started on port {} ({} I/O thread(s))",
                 endpoint.port(), threads_count);
    }

    void Stop()
    {
        if (!running.exchange(false))
            return;

        boost::system::error_code ec;
        acceptor.close(ec);
        io_ctx.stop();

        for (auto& t : threads)
            if (t.joinable())
                t.join();

        threads.clear();
    }

private:

    void DoAccept()
    {
        acceptor.async_accept(
            [this](boost::system::error_code ec, tcp::socket peer)
            {
                if (!ec && running.load())
                {
                    boost::system::error_code optEc;
                    peer.set_option(tcp::no_delay(true), optEc);
                    std::make_shared<Connection>(std::move(peer), root_path, io_ctx)->Start();
                }

                if (running.load())
                    DoAccept();
            });
    }

    struct Connection : std::enable_shared_from_this<Connection>
    {
        Connection(tcp::socket sock, fs::path const& root,
                   boost::asio::io_context& ioc)
            : strand(ioc)
            , socket(std::move(sock))
            , root_path(root)
        {}

        void Start()
        {
            ReadRequest();
        }

        void ReadRequest()
        {
            header_data.clear();

            auto self = shared_from_this();
            boost::asio::async_read_until(
                socket,
                boost::asio::dynamic_buffer(header_data, MAX_HEADER_BYTES),
                "\r\n\r\n",
                boost::asio::bind_executor(strand,
                    [this, self](boost::system::error_code ec, std::size_t)
                    {
                        if (ec) return Close();
                        HandleRequest();
                    }));
        }

        void HandleRequest()
        {
            std::istringstream stream(header_data);

            std::string request_line;
            std::getline(stream, request_line);
            if (!request_line.empty() && request_line.back() == '\r')
                request_line.pop_back();

            std::string method, target, version;
            {
                std::istringstream rl(request_line);
                rl >> method >> target >> version;
            }

            bool isHead = (method == "HEAD");
            if (!isHead && method != "GET")
            {
                SC_LOG_WARN("custom.streamingclient", "Rejecting unsupported method '{}' for '{}'", method, target);
                return Close();
            }

            uintmax_t rangeStart = 0;
            uintmax_t rangeEnd   = 0;
            bool      hasRange   = false;
            bool keepAlive = (version != "HTTP/1.0");

            std::string hostHeader;

            std::string line;
            while (std::getline(stream, line))
            {
                if (line == "\r" || line.empty())
                    break;

                if (!line.empty() && line.back() == '\r')
                    line.pop_back();

                if (line.rfind("Range:", 0) == 0 || line.rfind("range:", 0) == 0)
                {
                    std::smatch m;
                    if (std::regex_search(line, m, s_rangeRe))
                    {
                        rangeStart = std::stoull(m[1]);

                        if (rangeStart >= MAX_FILE_SIZE)
                            return Close();

                        if (m[2].matched && !m[2].str().empty())
                            rangeEnd = std::stoull(m[2]);

                        hasRange = true;
                    }
                }
                else if (line.rfind("Connection:", 0) == 0 || line.rfind("connection:", 0) == 0)
                {
                    std::string val = line.substr(line.find(':') + 1);
                    std::transform(val.begin(), val.end(), val.begin(),
                        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

                    if (val.find("close") != std::string::npos)
                        keepAlive = false;
                    else if (val.find("keep-alive") != std::string::npos)
                        keepAlive = true;
                }
                else if (line.rfind("Host:", 0) == 0 || line.rfind("host:", 0) == 0)
                {
                    hostHeader = line.substr(line.find(':') + 1);
                    while (!hostHeader.empty() && hostHeader.front() == ' ')
                        hostHeader.erase(hostHeader.begin());
                }
            }

            if (target.empty())
                target = "/";

            auto q = target.find('?');
            if (q != std::string::npos)
                target.erase(q);

            while (!target.empty() && target.front() == '/')
                target.erase(target.begin());

            if (target.empty())
            {
                return Close();
            }

            if (target == "signaturefile")
            {
                SendText(g_signatureFileContent, "application/octet-stream", isHead, keepAlive);
                return;
            }

            if (target == "wowmfil")
            {
                std::string host = hostHeader.empty() ? "127.0.0.1" : hostHeader;
                std::ostringstream full;
                full << "version=1\n";
                full << "isTrial=0\n";
                full << "source=http://" << host << "/\n";
                full << "Data;0;" << g_wowMfilBuildNumber << ";0\n";
                full << "Data/enUS;0;" << g_wowMfilBuildNumber << ";0\n";
                full << "Data/frFR;0;" << g_wowMfilBuildNumber << ";0\n";
                full << g_wowMfilLines;
                SendText(full.str(), "text/plain", isHead, keepAlive);
                return;
            }

            fs::path full = SafeJoin(root_path, target);
            if (full.empty())
            {
                SC_LOG_WARN("custom.streamingclient", "Blocked path-traversal attempt: '{}'", target);
                return Close();
            }

            boost::system::error_code fsec;

            if (!fs::exists(full, fsec) || fsec || fs::is_directory(full, fsec))
            {
                SC_LOG_WARN("custom.streamingclient", "404 for target='{}' resolved path='{}' (exists={}, ec={})",
                         target, full.string(), fs::exists(full, fsec), fsec.message());
                Send404(isHead, keepAlive);
                return;
            }

            uintmax_t totalSize = fs::file_size(full, fsec);
            if (fsec || totalSize == 0 || totalSize > MAX_FILE_SIZE)
            {
                Send404(isHead, keepAlive);
                return;
            }

            uintmax_t start   = 0;
            uintmax_t end     = totalSize - 1;
            bool      partial = false;

            if (hasRange)
            {
                partial = true;
                start   = rangeStart;

                if (rangeEnd != 0 && rangeEnd < end)
                    end = rangeEnd;

                if (start > end || start >= totalSize)
                    return Close();
            }

            auto file = std::make_shared<std::ifstream>(full, std::ios::binary);
            if (!file->is_open())
            {
                Send404(isHead, keepAlive);
                return;
            }

            file->seekg(static_cast<std::streamoff>(start));
            if (file->fail())
            {
                Send404(isHead, keepAlive);
                return;
            }

            uintmax_t contentLength = end - start + 1;

            std::ostringstream hdr;
            if (partial)
            {
                hdr << "HTTP/1.1 206 Partial Content\r\n";
                hdr << "Content-Range: bytes " << start << "-" << end
                    << "/" << totalSize << "\r\n";
            }
            else
            {
                hdr << "HTTP/1.1 200 OK\r\n";
            }
            hdr << "Content-Length: "  << contentLength << "\r\n";
            hdr << "Content-Type: application/octet-stream\r\n";
            hdr << "Accept-Ranges: bytes\r\n";
            hdr << "Connection: " << (keepAlive ? "keep-alive" : "close") << "\r\n\r\n";

            auto self   = shared_from_this();
            auto header = std::make_shared<std::string>(hdr.str());

            boost::asio::async_write(socket,
                boost::asio::buffer(*header),
                boost::asio::bind_executor(strand,
                    [this, self, header, file, isHead, contentLength, keepAlive]
                    (boost::system::error_code ec2, std::size_t) mutable
                    {
                        if (ec2) return Close();
                        if (isHead) { if (keepAlive) ReadRequest(); else Close(); return; }
                        StreamFile(file, contentLength, keepAlive);
                    }));
        }

        void StreamFile(std::shared_ptr<std::ifstream> file, uintmax_t remaining, bool keepAlive)
        {
            if (remaining == 0)
            {
                if (keepAlive) ReadRequest(); else Close();
                return;
            }

            auto   self   = shared_from_this();
            size_t toRead = static_cast<size_t>(
                std::min<uintmax_t>(FILE_CHUNK_BYTES, remaining));

            auto chunk = std::make_shared<std::vector<char>>(toRead);
            file->read(chunk->data(), toRead);
            std::streamsize got = file->gcount();

            if (got <= 0 || file->bad())
                return Close();

            boost::asio::async_write(socket,
                boost::asio::buffer(chunk->data(), static_cast<size_t>(got)),
                boost::asio::bind_executor(strand,
                    [this, self, file, chunk, remaining, got, keepAlive]
                    (boost::system::error_code ec, std::size_t) mutable
                    {
                        if (ec) return Close();
                        StreamFile(file, remaining - static_cast<uintmax_t>(got), keepAlive);
                    }));
        }

        void HelloWorld(bool isHead)
        {
            SendText(
                "<html><body>"
                "<h1>StreamingClient is running</h1>"
                "</body></html>",
                "text/html", isHead, false);
        }

        void Send404(bool isHead, bool keepAlive)
        {
            SendText("Not Found", "text/plain", isHead, keepAlive, "404 Not Found");
        }

        void SendText(std::string const& body,
                      std::string const& type,
                      bool               isHead,
                      bool               keepAlive,
                      std::string        status = "200 OK")
        {
            std::ostringstream hdr;
            hdr << "HTTP/1.1 " << status << "\r\n";
            hdr << "Content-Type: "   << type        << "\r\n";
            hdr << "Content-Length: " << body.size() << "\r\n";
            hdr << "Connection: " << (keepAlive ? "keep-alive" : "close") << "\r\n\r\n";

            auto self    = shared_from_this();
            auto headers = std::make_shared<std::string>(hdr.str());
            auto b       = std::make_shared<std::string>(body);

            boost::asio::async_write(socket,
                boost::asio::buffer(*headers),
                boost::asio::bind_executor(strand,
                    [this, self, headers, b, isHead, keepAlive](auto ec, auto)
                    {
                        if (ec) return Close();
                        if (isHead) { if (keepAlive) ReadRequest(); else Close(); return; }
                        boost::asio::async_write(socket,
                            boost::asio::buffer(*b),
                            boost::asio::bind_executor(strand,
                                [this, self, b, keepAlive](auto ec2, auto)
                                {
                                    if (ec2) return Close();
                                    if (keepAlive) ReadRequest(); else Close();
                                }));
                    }));
        }

        void Close()
        {
            boost::system::error_code ec;
            socket.shutdown(tcp::socket::shutdown_both, ec);
            socket.close(ec);
        }

        boost::asio::io_context::strand strand;
        tcp::socket                     socket;
        std::string                     header_data;
        fs::path                        root_path;
    };

    boost::asio::io_context  io_ctx;
    tcp::acceptor            acceptor;
    tcp::endpoint            endpoint;
    fs::path                 root_path;
    unsigned int             threads_count;
    std::vector<std::thread> threads;
    std::atomic<bool>        running;
};

static std::unique_ptr<HttpServer> g_streamingServer;

class StreamingClient : public WorldScript
{
public:
    StreamingClient() : WorldScript("StreamingClient") {}

    void OnStartup() override
    {
        if (!sConfigMgr->GetBoolDefault("StreamingClient.Enabled", true))
        {
            SC_LOG_INFO("custom.streamingclient", "StreamingClient is disabled.");
            return;
        }

        std::string dataDir = sConfigMgr->GetStringDefault("DataDir", ".");
        fs::path    cdnPath = fs::path(dataDir) / "cdn";

        std::error_code ec;
        if (!fs::exists(cdnPath, ec) || !fs::is_directory(cdnPath, ec))
        {
            SC_LOG_ERROR("custom.streamingclient",
                      "CDN directory '{}' does not exist -- StreamingClient will not start.",
                      cdnPath.string());
            return;
        }

        unsigned int cfgPort = static_cast<unsigned int>(
            sConfigMgr->GetIntDefault("StreamingClient.Port", 1119));
        if (cfgPort == 0 || cfgPort > 65535)
        {
            SC_LOG_WARN("custom.streamingclient",
                     "Invalid StreamingClient.Port ({}), falling back to 1119.", cfgPort);
            cfgPort = 1119;
        }

        unsigned int threads = static_cast<unsigned int>(
            sConfigMgr->GetIntDefault("ThreadPool", 2));
        if (threads < 2)
            threads = 2;

        g_wowMfilBuildNumber = static_cast<unsigned int>(
            sConfigMgr->GetIntDefault("StreamingClient.BuildNumber", 12340));

        fs::path cachePath = fs::path(dataDir) / "streamingclient_hashcache.txt";

        g_shuttingDown = false;

        std::thread([cdnPath, cachePath, cfgPort, threads]()
        {
            SC_LOG_INFO("custom.streamingclient", "Building signature file in the background (hashing new/modified cdn content)...");
            BuiltManifests built = BuildSignatureFile(cdnPath, cachePath, g_wowMfilBuildNumber);

            if (g_shuttingDown.load())
                return;

            g_signatureFileContent = std::move(built.signatureFile);
            g_wowMfilLines         = std::move(built.wowMfilLines);

            g_streamingServer = std::make_unique<HttpServer>(
                static_cast<unsigned short>(cfgPort),
                cdnPath.string(),
                threads);

            if (g_shuttingDown.load())
                return;

            g_streamingServer->Start();

            SC_LOG_INFO("custom.streamingclient",
                     "Serving '{}' on port {} with {} I/O thread(s).",
                     cdnPath.string(), cfgPort, threads);
        }).detach();

        SC_LOG_INFO("custom.streamingclient", "StreamingClient startup continuing in the background; worldserver is not blocked.");
    }

    void OnShutdown() override
    {
        g_shuttingDown = true;

        if (g_streamingServer)
        {
            g_streamingServer->Stop();
            g_streamingServer.reset();
            SC_LOG_INFO("custom.streamingclient", "StreamingClient stopped.");
        }
    }
};

void AddSC_StreamingClient()
{
    new StreamingClient();
}
