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

#ifndef DISCIPLE_PHASE_MANAGER_H
#define DISCIPLE_PHASE_MANAGER_H

#include <cstdint>
#include <mutex>

class DisciplePhaseManager
{
public:
    static DisciplePhaseManager& Instance()
    {
        static DisciplePhaseManager instance;
        return instance;
    }

    uint32_t Acquire()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        for (uint32_t bit = 1; bit <= 30; ++bit)
        {
            uint32_t mask = (1u << bit);
            if (!(_used & mask))
            {
                _used |= mask;
                return mask;
            }
        }
        return 0;
    }

    void Release(uint32_t mask)
    {
        if (!mask)
            return;

        std::lock_guard<std::mutex> lock(_mutex);
        _used &= ~mask;
    }

private:
    DisciplePhaseManager() : _used(0) { }
    DisciplePhaseManager(DisciplePhaseManager const&) = delete;
    DisciplePhaseManager& operator=(DisciplePhaseManager const&) = delete;

    std::mutex _mutex;
    uint32_t _used;
};

#endif // DISCIPLE_PHASE_MANAGER_H
