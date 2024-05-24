#pragma once

#include <App.hpp>

#include <cmath>
#include <stdio.h>

template <typename real>
class MVec3;

using MVec3f32 = MVec3<f32>;
using MVec3f64 = MVec3<f64>;

// TODO: Implementation of CGA for 3D space
template <typename real>
class MVec3
{
public:
    MVec3() {}

private:
    union
    {
        real data[32];
        struct
        {
            // Grade 0
            real e0 = 0;

            // Grade 1
            real e1 = 0, e2 = 0, e3 = 0, ep = 0, en = 0;

            // Grade 2
            real e12 = 0, e13 = 0, e1p = 0, e1n = 0, e23 = 0, e2p = 0, e2n = 0, e3p = 0, e3n = 0, epn = 0;

            // Grade 3
            real e123 = 0, e12p = 0, e12n = 0, e13p = 0, e13n = 0, e1pn = 0, e23p = 0, e23n = 0, e2pn = 0, e3pn = 0;

            // Grade 4
            real e123p = 0, e123n = 0, e12pn = 0, e13pn = 0, e23pn = 0;

            // Grade 5
            real e123pn = 0;
        };
    };
};