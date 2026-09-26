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

#include <boost/asio.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <memory>
#include <vector>
#include <thread>
#include <atomic>
#include <regex>
#include <string>

namespace fs = std::filesystem;
using boost::asio::ip::tcp;

static constexpr size_t   MAX_HEADER_BYTES = 8192;
static constexpr uintmax_t MAX_FILE_SIZE   = 5ULL * 1024 * 1024 * 1024;
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
        if (ec) { SC_LOG_ERROR("streamingclient", "acceptor open: {}", ec.message()); return; }

        acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);

        acceptor.bind(endpoint, ec);
        if (ec) { SC_LOG_ERROR("streamingclient", "acceptor bind: {}", ec.message()); return; }

        acceptor.listen(ACCEPT_BACKLOG, ec);
        if (ec) { SC_LOG_ERROR("streamingclient", "acceptor listen: {}", ec.message()); return; }

        DoAccept();

        for (unsigned i = 0; i < threads_count; ++i)
            threads.emplace_back([this]() { io_ctx.run(); });

        SC_LOG_INFO("streamingclient", "HTTP server started on port {} ({} I/O thread(s))",
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
                return Close();

            uintmax_t rangeStart = 0;
            uintmax_t rangeEnd   = 0;
            bool      hasRange   = false;

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
                // Decommenter pour verifier que le serveur tourne @ http://localhost:1119
                //HelloWorld(isHead);
                return;
            }

            if (target == "signaturefile")
            {
                SendText("", "application/octet-stream", isHead);
                return;
            }

            fs::path full = SafeJoin(root_path, target);
            if (full.empty())
            {
                SC_LOG_WARN("streamingclient", "Blocked path-traversal attempt: '{}'", target);
                return Close();
            }

            boost::system::error_code fsec;

            if (!fs::exists(full, fsec) || fsec || fs::is_directory(full, fsec))
            {
                Send404(isHead);
                return;
            }

            uintmax_t totalSize = fs::file_size(full, fsec);
            if (fsec || totalSize == 0 || totalSize > MAX_FILE_SIZE)
            {
                Send404(isHead);
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
                Send404(isHead);
                return;
            }

            file->seekg(static_cast<std::streamoff>(start));
            if (file->fail())
            {
                Send404(isHead);
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
            hdr << "Connection: close\r\n\r\n";

            auto self   = shared_from_this();
            auto header = std::make_shared<std::string>(hdr.str());

            boost::asio::async_write(socket,
                boost::asio::buffer(*header),
                boost::asio::bind_executor(strand,
                    [this, self, header, file, isHead, contentLength]
                    (boost::system::error_code ec2, std::size_t) mutable
                    {
                        if (ec2) return Close();
                        if (isHead) return Close();
                        StreamFile(file, contentLength);
                    }));
        }

        void StreamFile(std::shared_ptr<std::ifstream> file, uintmax_t remaining)
        {
            if (remaining == 0)
                return Close();

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
                    [this, self, file, chunk, remaining, got]
                    (boost::system::error_code ec, std::size_t) mutable
                    {
                        if (ec) return Close();
                        StreamFile(file, remaining - static_cast<uintmax_t>(got));
                    }));
        }

        void HelloWorld(bool isHead)
        {
            SendText(
                "<html><body>"
                "<h1>StreamingClient is running</h1>"
                "</body></html>",
                "text/html", isHead);
        }

        void Send404(bool isHead)
        {
            SendText("Not Found", "text/plain", isHead, "404 Not Found");
        }

        void SendText(std::string const& body,
                      std::string const& type,
                      bool               isHead,
                      std::string        status = "200 OK")
        {
            std::ostringstream hdr;
            hdr << "HTTP/1.1 " << status << "\r\n";
            hdr << "Content-Type: "   << type        << "\r\n";
            hdr << "Content-Length: " << body.size() << "\r\n";
            hdr << "Connection: close\r\n\r\n";

            auto self    = shared_from_this();
            auto headers = std::make_shared<std::string>(hdr.str());
            auto b       = std::make_shared<std::string>(body);

            boost::asio::async_write(socket,
                boost::asio::buffer(*headers),
                boost::asio::bind_executor(strand,
                    [this, self, headers, b, isHead](auto ec, auto)
                    {
                        if (ec) return Close();
                        if (isHead) return Close();
                        boost::asio::async_write(socket,
                            boost::asio::buffer(*b),
                            boost::asio::bind_executor(strand,
                                [this, self, b](auto, auto) { Close(); }));
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
            SC_LOG_INFO("streamingclient", "StreamingClient is disabled via config.");
            return;
        }

        std::string dataDir = sConfigMgr->GetStringDefault("DataDir", ".");
        fs::path    cdnPath = fs::path(dataDir) / "cdn";

        std::error_code ec;
        if (!fs::exists(cdnPath, ec) || !fs::is_directory(cdnPath, ec))
        {
            SC_LOG_ERROR("streamingclient",
                      "CDN directory '{}' does not exist -- StreamingClient will not start.",
                      cdnPath.string());
            return;
        }

        unsigned int cfgPort = static_cast<unsigned int>(
            sConfigMgr->GetIntDefault("StreamingClient.Port", 1119));
        if (cfgPort == 0 || cfgPort > 65535)
        {
            SC_LOG_WARN("streamingclient",
                     "Invalid StreamingClient.Port ({}), falling back to 1119.", cfgPort);
            cfgPort = 1119;
        }

        unsigned int threads = static_cast<unsigned int>(
            sConfigMgr->GetIntDefault("ThreadPool", 2));
        if (threads < 2)
            threads = 2;

        g_streamingServer = std::make_unique<HttpServer>(
            static_cast<unsigned short>(cfgPort),
            cdnPath.string(),
            threads);

        g_streamingServer->Start();

        SC_LOG_INFO("streamingclient",
                 "Serving '{}' on port {} with {} I/O thread(s).",
                 cdnPath.string(), cfgPort, threads);
    }

    void OnShutdown() override
    {
        if (g_streamingServer)
        {
            g_streamingServer->Stop();
            g_streamingServer.reset();
            SC_LOG_INFO("streamingclient", "StreamingClient stopped.");
        }
    }
};

void AddSC_StreamingClient()
{
    new StreamingClient();
}
