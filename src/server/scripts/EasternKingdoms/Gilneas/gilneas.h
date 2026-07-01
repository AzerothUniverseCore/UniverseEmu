/*
* This file is part of the AzerothUniverseCore Project. See AUTHORS file for Copyright information
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

#ifndef __GILNEAS_M_
#define __GILNEAS_M_

#include "Position.h"

namespace Gilneas
{

/*######
## Quest 14293 - Save Krennan Aranas
######*/

uint32 const pathSize1 = 7;
Position const greymanesHorsePath1[pathSize1] =
{
    { -1797.425f, 1396.941f, 20.07336f },
    { -1788.675f, 1378.441f, 20.07336f },
    { -1780.925f, 1368.941f, 20.07336f },
    { -1767.425f, 1358.191f, 19.82336f },
    { -1746.425f, 1358.691f, 20.07336f },
    { -1726.175f, 1354.191f, 19.82336f },
    { -1709.064f, 1348.535f, 19.78232f }
};

uint32 const pathSize2 = 13;
Position const greymanesHorsePath2[] =
{
    { -1664.807f, 1345.011f, 15.48499f },
    { -1662.807f, 1354.511f, 15.48499f },
    { -1667.307f, 1362.511f, 15.48499f },
    { -1674.307f, 1363.761f, 15.48499f },
    { -1686.057f, 1355.011f, 15.48499f },
    { -1691.057f, 1347.261f, 15.48499f },
    { -1705.807f, 1350.011f, 19.98499f },
    { -1731.307f, 1360.011f, 19.98499f },
    { -1744.807f, 1370.511f, 20.23499f },
    { -1758.807f, 1389.511f, 19.98499f },
    { -1768.307f, 1410.011f, 19.98499f },
    { -1771.557f, 1423.011f, 19.98499f },
    { -1770.955f, 1430.332f, 19.83506f }
};
}

#endif // __GILNEAS_M_
