#include <gtest/gtest.h>
#define MOCK_MESSAGE_QUEUE
#include "trike_message_concurrent_queue.h"
#include "trike_message_concurrent_queue.c"


class MessageConcurrentQueueTest : public testing::Test{
protected:
    void SetUp(){
        this->c_queue=trike_new_queue_c();
    };
    void TearDown(){
        trike_free_queue_c(this->c_queue);
    };

    trike_concurrent_queue_t *c_queue;
};

TEST_F(MessageConcurrentQueueTest,InitTest){
    EXPECT_EQ(trike_poll_queue_c(this->c_queue),0);
    EXPECT_TRUE(this->c_queue!=NULL);
}

TEST_F(MessageConcurrentQueueTest,PopEmptyTest){
    EXPECT_TRUE(trike_pop_task_c(this->c_queue)==NULL);
}

TEST_F(MessageConcurrentQueueTest,PushPopTest){
    EXPECT_EQ(trike_push_task_c(this->c_queue,NULL),0);
    EXPECT_TRUE(trike_pop_task_c(this->c_queue)==NULL);
}

TEST_F(MessageConcurrentQueueTest,PollFailTest){
    EXPECT_EQ(trike_poll_queue_c(NULL),-1);
}

TEST_F(MessageConcurrentQueueTest,PopFailTest){
    EXPECT_TRUE(trike_pop_task_c(NULL)==NULL);
}

TEST_F(MessageConcurrentQueueTest,PushFailTest){
    EXPECT_EQ(trike_push_task_c(NULL,NULL),-1);
}

int main(int argc,char *argv[]){
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
