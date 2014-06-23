#include <gtest/gtest.h>
#include <string.h>

#include "trike_zmq_worker_api.h"
#include "trike_msg_api.h"
#include "trike_gen_cc_rr.h"

#define MULTI_COUNT 100

#define MAX_ADDR_SIZE 255

volatile int response_count=0;
trike_transport_context_t *server_context;
char global_user_data[MAX_ADDR_SIZE];

#define WAIT_COUNTER(CUR_COUNT,COUNT) { while(CUR_COUNT<COUNT){}};

int handle_request(trike_transport_context_t *transport_context,trike_cc_rr_msg_t *msg);
int handle_response(trike_transport_context_t *transport_context,trike_cc_rr_msg_t *msg);

class CCRRTest : public testing::Test{
protected:
    void SetUp(){
        this->worker_context=start_worker(NULL);
        strcpy(this->addr,"tcp://127.0.0.1");
        strcpy(this->bind_addr,"tcp://*");
        this->req_port=50500;
        this->rep_port=50502;
        this->count=MULTI_COUNT;
        strcpy(this->user_data,"user_data");
        strcpy(global_user_data,this->user_data);
    };
    void TearDown(){
        stop_worker(this->worker_context);
    };
    char addr[MAX_ADDR_SIZE];
    char bind_addr[MAX_ADDR_SIZE];
    char user_data[MAX_ADDR_SIZE];
    int req_port;
    int rep_port;
    trike_worker_context_t *worker_context;
    int count;
};

TEST_F(CCRRTest,ReqRepTest){
    trike_transport_context_t *client_context;
    trike_gen_cc_reply_t reply;
    trike_gen_cc_request_t request;
    trike_cc_rr_msg_t trike_cc_rr_msg;
    char buf[]="test";
    // Add server
    reply.handle_request=handle_request;
    reply.req_port=this->req_port;
    reply.rep_port=this->rep_port;
    reply.addr=this->bind_addr;
    reply.user_data=this->user_data;
    server_context=add_rr_server(this->worker_context,reply);
    // Add client
    request.handle_reply=handle_response;
    request.req_port=this->req_port;
    request.rep_port=this->rep_port;
    request.addr=this->addr;
    client_context=add_rr_client(this->worker_context,request);
    // Check add result
    ASSERT_TRUE(server_context!=NULL);
    ASSERT_TRUE(client_context!=NULL);
    // Send requests
    for(int i=0;i<this->count;i++){
        trike_cc_rr_msg.dialog_id_data=buf;
	trike_cc_rr_msg.dialog_id_size=sizeof(buf);
        trike_cc_rr_msg.request_id=i;
        trike_cc_rr_msg.size=strlen(buf);
        trike_cc_rr_msg.data=buf;
        int send_result=trike_send_cc_rr_msg(client_context,trike_cc_rr_msg);
        ASSERT_EQ(send_result,0);
    }
    // Wait for request
    WAIT_COUNTER(response_count,this->count);
}

int handle_request(trike_transport_context_t *reply_transport_context,trike_cc_rr_msg_t *msg){
    char buf[]="response";
    trike_cc_rr_msg_t trike_cc_rr_msg;
    trike_cc_rr_msg.dialog_id_data=msg->dialog_id_data;
    trike_cc_rr_msg.dialog_id_size=msg->dialog_id_size;
    trike_cc_rr_msg.request_id=msg->request_id;
    trike_cc_rr_msg.size=sizeof(buf);
    trike_cc_rr_msg.data=buf;
    trike_send_cc_rr_msg(reply_transport_context,trike_cc_rr_msg);
    return 0;
}

int handle_response(trike_transport_context_t *transport_context,trike_cc_rr_msg_t *msg){
    response_count++;
    return 0;
}

int main(int argc,char *argv[]){
    ::testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
