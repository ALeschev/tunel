#include <gtest/gtest.h>
#define TESTING
#include "trike_message_queue.h"
#include "trike_message_queue.c"

#define MULTI_COUNT 1000000

class MessageQueueTest : public testing::Test{
protected:
    void SetUp(){this->queue=trike_new_queue();count=MULTI_COUNT;};
    void TearDown(){trike_free_queue(this->queue);};

    trike_queue_t *queue;
    int count;
};


TEST_F(MessageQueueTest,InitTest){
    EXPECT_EQ(trike_poll_queue(this->queue),0);
    EXPECT_TRUE(this->queue!=NULL);
}

TEST_F(MessageQueueTest,FindEmptyTail){
    EXPECT_TRUE(trike_find_tail(this->queue)==NULL);
}

TEST_F(MessageQueueTest,PopEmptyTest){
    EXPECT_TRUE(trike_pop_task(this->queue)==NULL);
}

TEST_F(MessageQueueTest,PopFailTest){
    EXPECT_TRUE(trike_pop_task(NULL)==NULL);
}

TEST_F(MessageQueueTest,PushFailTest){
    EXPECT_EQ(trike_push_task(NULL,NULL),-1);
}

TEST_F(MessageQueueTest,PollFailTest){
    EXPECT_EQ(trike_poll_queue(NULL),-1);
}

TEST_F(MessageQueueTest,PushTest){
    char buf[]="test_message";
    EXPECT_EQ(trike_push_task(this->queue,buf),0);
    EXPECT_EQ(trike_poll_queue(this->queue),1);
}

TEST_F(MessageQueueTest,PushPopTest){
    char buf[]="test_message";
    EXPECT_EQ(trike_push_task(this->queue,buf),0);
    EXPECT_STREQ((char*)trike_pop_task(this->queue),buf);
}

TEST_F(MessageQueueTest,PushPopOrderTest){
    char buf_1[]="test_message_1";
    char buf_2[]="test_message_2";
    EXPECT_EQ(trike_push_task(this->queue,buf_1),0);
    EXPECT_EQ(trike_push_task(this->queue,buf_2),0);
    EXPECT_STREQ((char*)trike_pop_task(this->queue),buf_1);
    EXPECT_STREQ((char*)trike_pop_task(this->queue),buf_2);
}


TEST_F(MessageQueueTest,MultiPushPopTest){
    char buf[]="test_message";
    for(int i=0;i<this->count;i++){
        ASSERT_EQ(trike_push_task(this->queue,buf),0);
        EXPECT_EQ(trike_poll_queue(this->queue),1);
        ASSERT_STREQ((char*)trike_pop_task(this->queue),buf);
    }
}

TEST_F(MessageQueueTest,MultiSeqPushPopTest){
    char buf[]="test_message";
    for(int i=0;i<this->count;i++){
        ASSERT_EQ(trike_push_task(this->queue,buf),0);
        EXPECT_EQ(trike_poll_queue(this->queue),i+1);
    }
    for(int i=0;i<this->count;i++){
        ASSERT_STREQ((char*)trike_pop_task(this->queue),buf);
        EXPECT_EQ(trike_poll_queue(this->queue),count-i-1);
    }
}

int main(int argc,char *argv[]){
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
