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

#include "tc_catch2.h"

#include "CompilerDefs.h"
#include "UniqueTrackablePtr.h"

struct TestObj
{
    TestObj(bool* deleted = nullptr) : Deleted(deleted) { }

    virtual ~TestObj()
    {
        if (Deleted)
            *Deleted = true;
    }

    bool* Deleted = nullptr;
};

struct TestObj2
{
    virtual ~TestObj2() = default;

    int a = 5;
};

struct TestObj3 : public TestObj2, public TestObj
{
};

struct TestObj4 : public TestObj
{
};

TEST_CASE("Syphrena::unique_trackable_ptr frees memory", "[UniqueTrackablePtr]")
{
    bool deleted = false;

    SECTION("reassigning new object deletes old one")
    {
        Syphrena::unique_trackable_ptr<TestObj> ptr = Syphrena::make_unique_trackable<TestObj>(&deleted);

        ptr.reset(new TestObj());

        REQUIRE(deleted == true);
    }

    SECTION("going out of scope deletes object")
    {
        REQUIRE(deleted == false);

        {
            Syphrena::unique_trackable_ptr<TestObj> ptr = Syphrena::make_unique_trackable<TestObj>(&deleted);
        }

        REQUIRE(deleted == true);
    }
}

TEST_CASE("Syphrena::unique_weak_ptr", "[UniqueTrackablePtr]")
{
    Syphrena::unique_trackable_ptr<int> ptr = Syphrena::make_unique_trackable<int>();

    Syphrena::unique_weak_ptr<int> weakRef = ptr;

    SECTION("when unique_trackable_ptr no longer holds a value then weak cannot retrieve it")
    {
        ptr.reset();

        REQUIRE(weakRef.expired());
        REQUIRE(!weakRef.lock());
    }

    SECTION("when unique_trackable_ptr is reassigned then weak cannot retrieve old value")
    {
        ptr.reset(new int);

        Syphrena::unique_weak_ptr<int> weakRef2 = ptr;

        REQUIRE(weakRef.expired());
        REQUIRE(!weakRef2.expired());
        REQUIRE(weakRef.lock() != weakRef2.lock());
    }

    SECTION("when unique_trackable_ptr holds a value then weak can retrieve it")
    {
        REQUIRE(!weakRef.expired());
        REQUIRE(!!weakRef.lock());
    }
}

// disable warning about invalid reinterpret_cast, test intentionally tests this
#ifdef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreinterpret-base-class"
#endif

TEST_CASE("Syphrena::unique_strong_ref_ptr type casts", "[UniqueTrackablePtr]")
{
    Syphrena::unique_trackable_ptr<TestObj> ptr = Syphrena::make_unique_trackable<TestObj3>();

    Syphrena::unique_weak_ptr<TestObj> weak = ptr;

    Syphrena::unique_strong_ref_ptr<TestObj> temp = weak.lock();
    REQUIRE(temp != nullptr);

    SECTION("static_pointer_cast")
    {
        Syphrena::unique_strong_ref_ptr<TestObj3> testObj2 = Syphrena::static_pointer_cast<TestObj3>(temp);

        REQUIRE(testObj2.get() == static_cast<TestObj3*>(ptr.get()));

        // sanity check that we didn't accidentally setup inheritance of TestObjs incorrectly
        REQUIRE(testObj2.get() != reinterpret_cast<TestObj3*>(ptr.get()));

        REQUIRE(testObj2 == Syphrena::static_pointer_cast<TestObj3>(weak).lock());
    }

    SECTION("reinterpret_pointer_cast")
    {
        Syphrena::unique_strong_ref_ptr<TestObj3> testObj2 = Syphrena::reinterpret_pointer_cast<TestObj3>(temp);

        REQUIRE(testObj2.get() == reinterpret_cast<TestObj3*>(ptr.get()));

        REQUIRE(testObj2 == Syphrena::reinterpret_pointer_cast<TestObj3>(weak).lock());
    }

    SECTION("succeeding dynamic_pointer_cast")
    {
        Syphrena::unique_strong_ref_ptr<TestObj3> testObj2 = Syphrena::dynamic_pointer_cast<TestObj3>(temp);

        REQUIRE(testObj2.get() == dynamic_cast<TestObj3*>(ptr.get()));

        REQUIRE(testObj2 == Syphrena::dynamic_pointer_cast<TestObj3>(weak).lock());
    }

    SECTION("failing dynamic_pointer_cast")
    {
        Syphrena::unique_strong_ref_ptr<TestObj4> testObj2 = Syphrena::dynamic_pointer_cast<TestObj4>(temp);

        REQUIRE(testObj2 == nullptr);

        REQUIRE(testObj2 == Syphrena::dynamic_pointer_cast<TestObj4>(weak).lock());
    }
}

#ifdef __clang__
#pragma GCC diagnostic pop
#endif
