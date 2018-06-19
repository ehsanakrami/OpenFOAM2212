/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2018 OpenCFD Ltd.
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "autoPtr.H"
#include "labelList.H"
#include "ListOps.H"
#include "IOstreams.H"
#include "Switch.H"

#include "C7H16.H"

using namespace Foam;


// An example of bad use, since our autoPtr is too generous when being passed
// around
void testTransfer1(autoPtr<labelList> ap)
{
    // Passed in copy, so automatically removes content
    // Transfer would be nice, but not actually needed

    Info<< "recv " << Switch(ap.valid()).c_str() << nl;
}


// An example of good use. We are allowed to manage the memory (or not)
// and not automatically start losing things.
void testTransfer2(autoPtr<labelList>&& ap)
{
    // As rvalue, so this time we actually get to manage content
    Info<< "recv " << Switch(ap.valid()).c_str() << nl;
}


// Constructor from literal nullptr is implicit
template<class T>
autoPtr<T> testNullReturn1()
{
    return nullptr;
}


// Constructor from raw pointer is explicit
template<class T>
autoPtr<T> testNullReturn2()
{
    T* p = new T;

    return p;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
//  Main program:

int main(int argc, char *argv[])
{
    {
        auto list = autoPtr<labelList>::New(10, label(-1));

        Info<<"create: " << list() << nl;

        Info<<"create: " << autoPtr<labelList>::New(10, label(-1))()
            << nl << nl;
    }

    // Confirm that forwarding with move construct actually works as expected
    {
        auto source = identity(8);
        Info<<"move construct from "
            << flatOutput(source) << " @ " << long(source.cdata())
            << nl << nl;

        auto list = autoPtr<labelList>::New(std::move(source));

        Info<<"created: " << flatOutput(*list) << " @ " << long(list->cdata())
            << nl << nl;

        Info<<"orig: "
            << flatOutput(source) << " @ " << long(source.cdata())
            << nl << nl;
    }

    // Explicit construct Base from Derived
    {
        autoPtr<liquidProperties> liqProp
        (
            autoPtr<C7H16>::New()
        );

        Info<<"liq 1: " << liqProp() << nl << nl;
    }

    // Construct Base from Derived
    {
        autoPtr<liquidProperties> liqProp =
            autoPtr<liquidProperties>::NewFrom<C7H16>();

        Info<<"liq 2: " << liqProp() << nl << nl;
    }

    // Construct Base from Derived
    {
        const autoPtr<liquidProperties> liqProp(autoPtr<C7H16>::New());

        Info<<"liq: " << liqProp() << nl << nl;
        Info<<"liq-type: " << liqProp->type() << nl << nl;
        Info<<"type: " << typeid(liqProp.get()).name() << nl;
    }

    // Memory transfer
    {
        Info<< nl << nl;

        auto list = autoPtr<labelList>::New(identity(8));
        Info<<"forward to function from "
            << flatOutput(*list) << " @ " << long(list->cdata())
            << nl << nl;

        testTransfer2(std::move(list));

        Info<<"now have valid=" << Switch(list.valid()).c_str();

        if (list.valid())
        {
            Info<< nl
                << flatOutput(*list) << " @ " << long(list->cdata())
                << nl;
        }
        else
        {
            Info<< nl;
        }
    }

    // Memory transfer
    {
        Info<< nl << nl;

        testTransfer2(autoPtr<labelList>::New(identity(8)));
    }

    // Memory transfer
    {
        Info<< nl << nl;

        auto list = autoPtr<labelList>::New(identity(8));
        Info<<"forward to function from "
            << flatOutput(*list) << " @ " << long(list->cdata())
            << nl << nl;

        testTransfer2(std::move(list));

        Info<<"now have valid=" << Switch(list.valid()).c_str();

        if (list.valid())
        {
            Info<< nl
                << flatOutput(*list) << " @ " << long(list->cdata())
                << nl;
        }
        else
        {
            Info<< nl;
        }
    }


    // Memory transfer
    {
        auto ptr1 = autoPtr<labelList>::New();
        auto ptr2 = autoPtr<labelList>::New();

        Info<<"ptr valid: " << ptr1.valid() << nl;

        // Refuses to compile (good!):   ptr1 = new labelList(10);

        // Does compile (good!):   ptr1 = nullptr;

        ptr1.reset(std::move(ptr2));
    }


    {
        // Good this work:
        autoPtr<labelList> ptr1 = testNullReturn1<labelList>();

        // Good this does not compile:
        // autoPtr<labelList> ptr2 = testNullReturn2<labelList>();
    }


    return 0;
}


// ************************************************************************* //