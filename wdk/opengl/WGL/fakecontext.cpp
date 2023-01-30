
#include <map>
#include <cassert>

#include "wdk/opengl/WGL/fakecontext.h"

namespace wgl
{

std::map<const wdk::Config*, std::weak_ptr<FakeContext>> gFakes;

void StashFakeContext(const wdk::Config* conf, std::shared_ptr<FakeContext> ctx)
{
    gFakes[conf] = ctx;
}

void FetchFakeContext(const wdk::Config* conf, std::shared_ptr<FakeContext>& ctx)
{
    auto it = gFakes.find(conf);
    assert(it != gFakes.end());

    std::weak_ptr<FakeContext>& weak = (*it).second;
    assert(!weak.expired());

    ctx = weak.lock();
}

void EraseFakeContext(const wdk::Config* conf)
{
    gFakes.erase(conf);
}

} // namespace