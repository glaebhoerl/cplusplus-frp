#include "FRP.h"

using namespace glaebhoerl;

int main(int, char**)
{
    Events<> e0; Events<int> e1; Events<bool, int> e2; Variable<int> v; Variable<int*> va;
    auto vf = constant([] (int x) { return x * 9; });
    auto vb = constant([] (int x) { return x > 9; });
    Events<Events<int>> ee;
    Variable<Variable<int>> vv;
    Events<void (*)()> ex;
    Variable<Events<>> ve;

    {
        Events<int> a(e1);
        Events<int> b = e1.mergedWith(a);
        Events<bool> c = e2.transformedBy([] (bool b, int n) { return b || n > 9; });
        Events<int> d = b.transformedBy(vf);
        Events<bool, int> e = e2.filteredBy([] (bool b, int) { return b; });
        //Events<bool, int> f = e.filteredBy(v.transformedBy([] (int n) { return n < 0; }));
        //Events<int> g = e1.filteredBy(vb);
        std::array<Events<int>, 4> h = e1.partitionedBy<4>([] (int n) { return n % 4; });
        Events<bool> i = e0.taggedWith(false);
        Events<bool, int> j = e1.taggedAtFrontWith(true);
        Events<int> k = e0.withSnapshotOf(v);
        Events<int, int> l = k.withSnapshotAtFrontOf(v);
        Events<int, bool, bool, bool> m = e2.positions<1, 0, 0, 0>();
        Events<std::tuple<bool, int>> n = e2.tupled();
        Events<bool, int> o = n.untupled();
        Events<int> p = ee.flattened();
        Variable<int> q = e1.lastValueStartingWith(0);
        Variable<std::array<int, 5>> r = e1.historyStartingWithDefaults<5>();
        Variable<std::vector<int>> s = e1.history(9);
        Variable<int> t = e2.count();
        ex.exec();
        Events<int> u = r.changes()[4];
        Events<int> w = r.changes()[v];
        Events<bool> x = vb.changes()(9);
        Events<bool> y = vb.changes()(v);
        Events<int> z = u * 9 - v;
    }

    {
        Variable<int> a(5);
        Variable<int> b(5, e1);
        Variable<int> c(b);
        Variable<bool> d = a.transformedBy([] (int n) { return n < 55; });
        Variable<bool> e = b.transformedBy(vb);
        Variable<int> f = vv.flattened();
        Variable<int> g = va[5];
        Variable<int> h = va[v];
        Variable<bool> i = vb(9);
        Variable<bool> j = vb(v);
        Variable<bool> k = (vf((a + b * c) << 9) < g) || i;
        Variable<std::tuple<bool, int, int>> l = tupled(d, b, a);
        Variable<int> m = sample([] () { return 99; }, e0);
        Events<> n = variableEvents(ve);
        Mutable<int> o;
        o /= 9;
    }
}