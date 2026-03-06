#include "stdin.hpp"

etl::pool<Arguments, STDIN_ARG_SLOTS> StdinArgPool::_pool;
SemaphoreHandle_t                    StdinArgPool::_mutex = nullptr;

bool StdinArgPool::init()
{
    _mutex = xSemaphoreCreateMutex();
    if (!_mutex) { log_e("StdinArgPool: mutex create failed"); return false; }
    return true;
}

Arguments *StdinArgPool::alloc()
{
    for (;;) {
        xSemaphoreTake(_mutex, portMAX_DELAY);
        void *mem = _pool.allocate();
        xSemaphoreGive(_mutex);
        if (mem) {
            return new (mem) Arguments{};
        }
        vTaskDelay(1);
    }
}

void StdinArgPool::free(Arguments *blk)
{
    if (!blk) return;
    blk->~Arguments();
    xSemaphoreTake(_mutex, portMAX_DELAY);
    _pool.release(blk);
    xSemaphoreGive(_mutex);
}

size_t StdinArgPool::available()
{
    xSemaphoreTake(_mutex, portMAX_DELAY);
    size_t n = _pool.available();
    xSemaphoreGive(_mutex);
    return n;
}

void Arguments::assign(int argc, const char *const *args)
{
    argv.clear();
    for (int i = 0; i < argc && i < LUA_MAX_ARGS; ++i)
        argv.emplace_back(args[i]);
}

int Stdin::readline(Line &out)
{
    out.clear();
    for (;;) {
        while (!Serial.available()) vTaskDelay(1);
        char c = static_cast<char>(Serial.read());
        if (c == '\r' || c == '\n') {
            Serial.print("\r\n");
            break;
        }
        if ((c == 127 || c == 8) && !out.empty()) {
            out.pop_back();
            Serial.print("\b \b");
        } else if (c >= 32 && c < 127 && !out.full()) {
            out.push_back(c);
            Serial.write(c);
        }
    }
    return static_cast<int>(out.size());
}

int Stdin::tokenise(Line &line, Arguments &blk)
{
    blk.argv.clear();
    etl::string_view view(line.c_str(), line.size());
    size_t i = 0;
    while (i < view.size() && !blk.argv.full()) {
        while (i < view.size() && (view[i] == ' ' || view[i] == '\t')) ++i;
        if (i >= view.size()) break;
        size_t start = i;
        while (i < view.size() && view[i] != ' ' && view[i] != '\t') ++i;
        blk.argv.emplace_back(view.substr(start, i - start));
    }
    return static_cast<int>(blk.argv.size());
}