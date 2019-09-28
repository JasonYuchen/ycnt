//
// Created by jason on 2019/9/25.
//

#include <unistd.h>

#include <ycnt/base/LogStream.h>
#include <ycnt/base/Thread.h>

#include <vector>

using namespace std;
using namespace ycnt;
using namespace ycnt::base;

std::atomic_int result(0);

void bench(CountDownLatch *latch)
{
  latch->wait();
  int cnt = 0;
  const int kBatch = 10000;
  Timestamp start = Timestamp::now();
  for (int i = 0; i < kBatch; ++i) {
    LOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz "
             << 123 << 456.7 << " =" << cnt;
    ++cnt;
  }
  Timestamp end = Timestamp::now();
  printf("%f\n", kBatch / timeDifference(end, start));
  result += kBatch / timeDifference(end, start);
}

int main(int argc, char **argv)
{
  int threadsNum = 1;
  int bucketsNum = 1;
  Logger::init("testLogger", bucketsNum);
  CountDownLatch latch(1);
  vector<Thread *> threads;

  for (int i = 0; i < threadsNum; ++i) {
    threads.emplace_back(new Thread{bind(bench, &latch)});
    threads.back()->start();
  }
  latch.countDown();
  for (int i = 0; i < threadsNum; ++i) {
    threads[i]->join();
  }
  sleep(3);
  printf("%d", result.load());
  return 0;
};