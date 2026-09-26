
# mod-streamingclient
Module that embeds a HTTP server into the worldserver process, allowing players to connect using a portable play-while-downloading client which is only a few megabytes, just like Cataclysm and onwards.
> **Disclaimer:** This mod is intended as a convenient, plug-and-play way to allow players connecting to your server. For production use, I'd recommend using dedicated webservers such as Nginx/Apache. This HTTP server has not been hardened for security or optimized for high performance. Use it at your risk or feel free to create a PR ;)

## Installation (Server)
1. Create a `cdn` folder inside your `DataDir` (the path set in `worldserver.conf`) and place your WoW `MPQ` files there like this:
```
DataDir\
│
├───Cameras
├───cdn
│   └───Data
│   │   common-2.MPQ
│   │   common.MPQ
│   │   expansion.MPQ
│   │   lichking.MPQ
│   │   patch-2.MPQ
│   │   patch-3.MPQ
│   │   patch.MPQ
│   │
│   └───enUS
│           backup-enUS.MPQ
│           base-enUS.MPQ
│           expansion-locale-enUS.MPQ
│           expansion-speech-enUS.MPQ
│           lichking-locale-enUS.MPQ
│           lichking-speech-enUS.MPQ
│           locale-enUS.MPQ
│           patch-enUS-2.MPQ
│           patch-enUS-3.MPQ
│           patch-enUS.MPQ
│           speech-enUS.MPQ
├───dbc
├───maps
├───mmaps
└───vmaps
```
2. Additionally port forward `1119` (or any other port configured in your `mod-streamingclient.conf`).

## Installation (Client)
3. Setup your folder like this to distribute to your players:
```
StreamingClient\
│   DivxDecoder.dll
│   Wow.exe
│   WoW.mfil
│
└───WTF\
        Config.wtf
```
**WTF/Config.wtf**:
```
SET realmlist "your.server.address"
SET locale "enUS"
```
**WoW.mfil**:
```
version=1
isTrial=0
source=http://yourcdn.com/
Data;0;12340;0
Data/enUS;0;12340;0
Data/common.MPQ;2881154862;12340;0
Data/common-2.MPQ;1810430636;12340;0
Data/expansion.MPQ;1921219911;12340;0
Data/lichking.MPQ;2553948549;12340;0
Data/patch.MPQ;4004713057;12340;0
Data/patch-2.MPQ;1401729059;12340;0
Data/patch-3.MPQ;605089137;12340;0
Data/enUS/base-enUS.MPQ;29176975;12340;0
Data/enUS/locale-enUS.MPQ;204291376;12340;0
Data/enUS/expansion-locale-enUS.MPQ;17389181;12340;0
Data/enUS/expansion-speech-enUS.MPQ;241033297;12340;0
Data/enUS/lichking-locale-enUS.MPQ;12354378;12340;0
Data/enUS/lichking-speech-enUS.MPQ;354008839;12340;0
Data/enUS/speech-enUS.MPQ;438430439;12340;0
Data/enUS/backup-enUS.MPQ;167245856;12340;0
Data/enUS/patch-enUS.MPQ;296616080;12340;0
Data/enUS/patch-enUS-2.MPQ;225570171;12340;0
Data/enUS/patch-enUS-3.MPQ;100373935;12340;0
```
> `sourcemanifest` and `transportmanifest` lines are **not** required.

## Known Issues
- Disconnects or game crashes may occur when moving at high speed or in high-density / graphically intensive areas.

## Copyright
License: GPL 2.0