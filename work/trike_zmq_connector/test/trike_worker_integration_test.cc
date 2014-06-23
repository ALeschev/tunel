#include <gtest/gtest.h>
#include <string.h>
#include "trike_zmq_worker_api.h"
#include "trike_msg_api.h"

#define MULTI_COUNT 1000000

#define MAX_ADDR_SIZE 255

volatile int response_count=0;

#define WAIT_COUNTER(COUNT,CUR_COUNT) { while(CUR_COUNT<COUNT){}; }

int handle_request(trike_transport_context_t *transport_context,void *data,int size);
int handle_response(trike_transport_context_t *transport_context,void *data,int size);

class WorkerTest : public testing::Test{
protected:
    void SetUp(){
        this->worker_context=start_worker(NULL);
        strcpy(this->addr,"tcp://127.0.0.1");
        this->port=50500;
        this->count=MULTI_COUNT;
    };
    void TearDown(){
        stop_worker(this->worker_context);
    };

    char addr[MAX_ADDR_SIZE];
    int port;
    trike_worker_context_t *worker_context;
    int count;
};

TEST_F(WorkerTest,ReqRepTest){
    trike_transport_context_t *connection_context,*listener_context;
    listener_context=add_listener(this->worker_context,handle_request,NULL,this->addr,this->port,ZMQ_REP);
    connection_context=add_connection(this->worker_context,handle_response,NULL,this->addr,this->port,ZMQ_REQ);
    ASSERT_TRUE(listener_context!=NULL);
    ASSERT_TRUE(connection_context!=NULL);
    char buf[]="test";
    for(int i=0;i<this->count;i++){
        int send_result=trike_send_msg(connection_context,buf,sizeof(buf));
        ASSERT_EQ(send_result,0);
    }
    WAIT_COUNTER(response_count,this->count);
}

int handle_request(trike_transport_context_t *transport_context,void *data,int size){
    char buf[]="response";
    trike_send_msg(transport_context,buf,9);
    return 0;
}


int handle_response(trike_transport_context_t *transport_context,void *data,int size){
    response_count++;
    return 0;
}

int main(int argc,char *argv[]){
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
