
#include <map>
#include <cassert>

#include "fakecontext.h"

namespace wgl
{

std::map<const wdk::config*, std::weak_ptr<FakeContext>> gFakes;

void stashFakeContext(const wdk::config* conf, std::shared_ptr<FakeContext> ctx)
{
    gFakes[conf] = ctx;
}

void fetchFakeContext(const wdk::config* conf, std::shared_ptr<FakeContext>& ctx)
{
    auto it = gFakes.find(conf);
    assert(it != gFakes.end());

    std::weak_ptr<FakeContext>& weak = (*it).second;
    assert(!weak.expired());

    ctx = weak.lock();
}

void eraseFakeContext(const wdk::config* conf)
{
    gFakes.erase(conf);
}

} // namespace