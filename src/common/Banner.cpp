/*
 * This file is part of the SyphrenaCore Project. See AUTHORS file for Copyright information
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
 */

#include "Banner.h"
#include "GitRevision.h"
#include "StringFormat.h"

void Syphrena::Banner::Show(char const* applicationName, void(*log)(char const* text), void(*logExtraInfo)())
{
    log(Syphrena::StringFormat("{} ({})", GitRevision::GetFullVersion(), applicationName).c_str());
    log(R"(<Ctrl-C> to stop.)" "\n");
    log(R"(   ________  ________  _______   ________  ________  _________  ___  ___)");
    log(R"(  |\   __  \|\_____  \|\  ___ \ |\   __  \|\   __  \|\___   ___\\  \|\  \)");
    log(R"(  \ \  \|\  \\|___/  /\ \   __/|\ \  \|\  \ \  \|\  \|___ \  \_\ \  \\\  \)");
    log(R"(   \ \   __  \   /  / /\ \  \_|/_\ \   _  _\ \  \\\  \   \ \  \ \ \   __  \)");
    log(R"(    \ \  \ \  \ /  /_/__\ \  \_|\ \ \  \\  \\ \  \\\  \   \ \  \ \ \  \ \  \)");
    log(R"(     \ \__\ \__\\________\ \_______\ \__\\ _\\ \_______\   \ \__\ \ \__\ \__\)");
    log(R"(      \|__|\|__|\|_______|\|_______|\|__|\|__|\|_______|    \|__|  \|__|\|__|)");
    log(R"(         ___  ___  ________   ___  ___      ___ _______   ________  ________  _______)");
    log(R"(        |\  \|\  \|\   ___  \|\  \|\  \    /  /|\  ___ \ |\   __  \|\   ____\|\  ___ \)");
    log(R"(        \ \  \\\  \ \  \\ \  \ \  \ \  \  /  / | \   __/|\ \  \|\  \ \  \___|\ \   __/|)");
    log(R"(         \ \  \\\  \ \  \\ \  \ \  \ \  \/  / / \ \  \_|/_\ \   _  _\ \_____  \ \  \_|/__)");
    log(R"(          \ \  \\\  \ \  \\ \  \ \  \ \    / /   \ \  \_|\ \ \  \\  \\|____|\  \ \  \_|\ \)");
    log(R"(           \ \_______\ \__\\ \__\ \__\ \__/ /     \ \_______\ \__\\ _\ ____\_\  \ \_______\)");
    log(R"(            \|_______|\|__| \|__|\|__|\|__|/       \|_______|\|__|\|__|\_________\|_______|)");
    log(R"(                                                    C O R E           \|_________|)");
    log(R"(	http://AzerothUniverseCore.org)" "\n");

    if (logExtraInfo)
        logExtraInfo();
}