#include <iostream>
#include <chrono>
#include <thread>
#include <gtest/gtest.h>

#include "execution_graph/logic_controllers/CacheMachine.h"  // WaitingQueue
#include "execution_graph/logic_controllers/LogicPrimitives.h"  // BlazingTable
#include "execution_graph/logic_controllers/BlazingColumn.h"  // BlazingColumn

#define DESCR(d) RecordProperty("description", d)

using namespace ral;


class WaitingQueueTest : public ::testing::Test {
protected:
   // void SetUp() override {}
   // void TearDown() override {}

   // Minimum required to create and return a cache::message instance for use
   // with a WaitingQueue.
   std::unique_ptr<cache::message>
   createCacheMsg(std::string msgId) {
      std::vector<std::unique_ptr<frame::BlazingColumn>> blazingColumns;
      std::vector<std::string> colNames = {};

      auto blazingTable = \
         std::make_unique<frame::BlazingTable>(std::move(blazingColumns),
                                               colNames);
      auto content = \
         std::make_unique<cache::GPUCacheData>(std::move(blazingTable));

      return std::move(std::make_unique<cache::message>(std::move(content),
                                                        msgId));
   }

   // Calls put() and passes msg on a WaitingQueue instance pointer after
   // waiting delayMs
   std::thread
   putCacheMsgAfter(int delayMs, cache::WaitingQueue* wqPtr,
                    std::unique_ptr<cache::message> msg) {
      auto worker = [](int delayMs, cache::WaitingQueue* wqPtr,
                       std::unique_ptr<cache::message> msg) {
         std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
         wqPtr->put(std::move(msg));
      };
      return std::thread(worker, delayMs, wqPtr, std::move(msg));
   }

   // Calls finish() on a WaitingQueue instance pointer after waiting delayMs
   std::thread
   callFinishAfter(int delayMs, cache::WaitingQueue* wqPtr) {
      auto worker = [](int delayMs, cache::WaitingQueue* wqPtr) {
         std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
         wqPtr->finish();
      };
      return std::thread(worker, delayMs, wqPtr);
   }

private:
};


TEST_F(WaitingQueueTest, putPop) {
   DESCR("simple test of single put-pop");

   cache::WaitingQueue wq;
   std::string msgId = "uniqueId1";

   wq.put(createCacheMsg(msgId));
   auto msgOut = wq.pop_or_wait();

   ASSERT_NE(msgOut, nullptr);
   EXPECT_EQ(msgOut->get_message_id(), msgId);
}


TEST_F(WaitingQueueTest, putPopSeveral) {
   DESCR("test of several put-pop operations ensuring proper ordering");

   cache::WaitingQueue wq;
   int numItems = 99;

   for(int i=0; i<numItems; ++i) {
      wq.put(createCacheMsg("uniqueId" + std::to_string(i)));
   }

   // Ensure msgs popped in correct order
   for(int i=0; i<numItems; ++i) {
      auto msgOut = wq.pop_or_wait();
      ASSERT_NE(msgOut, nullptr);
      EXPECT_EQ(msgOut->get_message_id(), "uniqueId" + std::to_string(i));
   }
}


TEST_F(WaitingQueueTest, putMultiThreadsPopAtEnd) {
   DESCR("test of several put operations from different threads "
         "asynchronously, ensuring all msgs processed");

   cache::WaitingQueue wq;
   int numItems = 99;

   // Create all the IDs upfront and use the vector for adding messages to a
   // WaitingQueue
   std::set<std::string> ids;
   for(int i=0; i<numItems; ++i) {
      ids.insert("uniqueId" + std::to_string(i));
   }
   ASSERT_EQ(ids.size(), numItems);

   // Create individual threads that each push an individual message on at a
   // random time in the future (10ms < time < 110ms)
   std::vector<std::thread> threads;
   for(auto &id : ids){
      auto msg = createCacheMsg(id);
      threads.push_back(putCacheMsgAfter(((rand() % 100) + 10), &wq,
                                         std::move(msg)));
   }
   ASSERT_EQ(threads.size(), numItems);

   // Ensure all messages are pushed before checking the WaitingQueue
   std::for_each(threads.begin(), threads.end(),
                 [](std::thread& t){ t.join(); });

   // Ensure every message was accounted for
   for(int i=0; i<numItems; ++i) {
      auto msgOut = wq.pop_or_wait();
      ASSERT_NE(msgOut, nullptr);
      auto idFound = ids.find(msgOut->get_message_id());
      EXPECT_NE(idFound, ids.end());
      if(idFound != ids.end()) {
         ids.erase(idFound);
      }
   }
   EXPECT_EQ(ids.size(), 0);

   // Ensure all messages were popped
   EXPECT_FALSE(wq.has_next_now());
}


TEST_F(WaitingQueueTest, putWaitForPop) {
   DESCR("ensures pop_or_wait() properly waits for a message");

   cache::WaitingQueue wq;
   std::string msgId = "uniqueId1";
   auto msg = createCacheMsg(msgId);

   // Put a message in the queue after waiting 300ms
   std::thread t = putCacheMsgAfter(300, &wq, std::move(msg));

   // This should wait until the message is present
   auto msgOut = wq.pop_or_wait();

   ASSERT_NE(msgOut, nullptr);
   EXPECT_EQ(msgOut->get_message_id(), msgId);
   t.join();  // should not be needed
}


TEST_F(WaitingQueueTest, DISABLED_putAndPopMultiThreads) {
   DESCR("test of several put-pop operations from different asynchronous "
         "threads simultaneously, ensuring all msgs processed");
   // Start out with a queue with some message_ptr already in it, run pop_or
   // wait on multiple threads and have another thread which is adding more data
   // to the queue while the other threads are pulling. You can add the messages
   // by either using the put or somehow manually changing the contents of the
   // dequeue in a lock safe way.
   // This uses both a short and long delay to ensure the timeout for logging is
   // being triggered.
}


TEST_F(WaitingQueueTest, putGet) {
   DESCR("simple test of single put-get");

   cache::WaitingQueue wq;
   std::string msgId = "uniqueId1";

   wq.put(createCacheMsg(msgId));
   auto msgOut = wq.get_or_wait(msgId);

   ASSERT_NE(msgOut, nullptr);
   EXPECT_EQ(msgOut->get_message_id(), msgId);

}


TEST_F(WaitingQueueTest, putGetWaitForId) {
   DESCR("ensures get waits for msg with proper ID");

   cache::WaitingQueue wq;
   std::string msgId1 = "uniqueId1";
   std::string msgId2 = "uniqueId2";

   // Put mgs1 in the queue immediately and msg2 after 300ms
   wq.put(createCacheMsg(msgId1));
   std::thread t = putCacheMsgAfter(300, &wq, createCacheMsg(msgId2));

   // msg with ID 2 won't show up for ~300ms, so this should wait
   auto msgOut = wq.get_or_wait(msgId2);
   ASSERT_NE(msgOut, nullptr);
   EXPECT_EQ(msgOut->get_message_id(), msgId2);

   // msg with ID 1 should still be present
   msgOut = wq.get_or_wait(msgId1);
   ASSERT_NE(msgOut, nullptr);
   EXPECT_EQ(msgOut->get_message_id(), msgId1);
   t.join();  // should not be needed
}


// FIXME: enable this test when
// https://github.com/BlazingDB/blazingsql/issues/884 is closed
TEST_F(WaitingQueueTest, DISABLED_putGetWaitForNonexistantId) {
   DESCR("ensures a get_or_wait() call on a non-existant ID can be cancelled");

   cache::WaitingQueue wq;
   std::string msgId1 = "uniqueId1";
   std::string msgId2 = "uniqueId2";

   wq.put(createCacheMsg(msgId1));

   // msg with ID 2 will never show up.
   // finish() will be called after 300ms, which should break get_or_wait() out
   // of its polling loop.
   std::thread t = callFinishAfter(300, &wq);
   auto msgOut = wq.get_or_wait(msgId2);    // FIXME: this will not return!

   ASSERT_NE(msgOut, nullptr);
   t.join();  // should not be needed
}


TEST_F(WaitingQueueTest, DISABLED_waitForNextMultiThreads) {
   DESCR("ensures wait_for_next() from multiple threads service all the msgs "
         "put asynchronously");
   // Start out with an empty dequeue. call wait_for_next on n threads. Add n
   // items to the dequeue (probably using put) and verify that all of the
   // wait_for_next threads complete and have one of the n messages you
   // inserted.
   // This uses both a short and long delay to ensure the timeout for logging is
   // being triggered.
}


TEST_F(WaitingQueueTest, DISABLED_waitUntilFinishedMultiThreads) {
   DESCR("test wait_until_finished() from multiple threads with a single "
         "separate thread calling finish() asynchronously");
   // start a few threads which call wait until finished. On a second thread
   // block execution for a few seconds then call the finish() function on
   // WaitingQueue.
   // This uses both a short and long delay to ensure the timeout for logging is
   // being triggered.
}


TEST_F(WaitingQueueTest, DISABLED_popUnsafe) {
   DESCR("ensures all msgs added are removed in fifo order using pop_unsafe()");
   // start out with n messages in the dequeue and call pop_unsafe n times and
   // make sure that you have the same messages back in a fifo order
}


TEST_F(WaitingQueueTest, DISABLED_getAllOrWaitMultiThreads) {
   DESCR("tests get_all_or_wait() from a thread then adding msgs and calling "
         "finish() from another");
   // start out with a queue that is empty. on one thread add n messages to it
   // then call on the WaitingQueue finish(). On another thread call
   // get_all_or_wait()
   // This uses both a short and long delay to ensure the timeout for logging is
   // being triggered.
}


TEST_F(WaitingQueueTest, DISABLED_getAllUnsafe) {
   DESCR("tests get_all_unsafe() by ensuring it returns all queued messages");
   // start out with a dequeue that contains n messages. call get_all_unsafe and
   // ensure you got all of the messages that were in the dequeue
}


TEST_F(WaitingQueueTest, DISABLED_putAllUnsafe) {
   DESCR("tests put_all_unsafe() by ensuring it queued all messages put");
}
