#include "cudf/types.h"
#include "utils/column_factory.h"
#include <blazingdb/transport/Node.h>
#include <blazingdb/transport/api.h>
#include <blazingdb/transport/io/reader_writer.h>
#include <chrono>
#include <cuda.h>

#include "Utils.cuh"
#include "communication/messages/ComponentMessages.h"
#include "communication/messages/GPUComponentMessage.h"
#include "communication/network/Client.h"
#include "communication/network/Server.h"
#include <memory>
#include <numeric>
#include <nvstrings/NVCategory.h>
#include <thread>
#include <from_cudf/cpp_tests/utilities/base_fixture.hpp>
#include <cudf/column/column_factories.hpp>
#include <from_cudf/cpp_tests/utilities/column_utilities.hpp>
#include <cudf/utilities/type_dispatcher.hpp>
#include <from_cudf/cpp_tests/utilities/type_lists.hpp>
#include <from_cudf/cpp_tests/utilities/column_wrapper.hpp>
#include <from_cudf/cpp_tests/utilities/legacy/cudf_test_utils.cuh>
#include <from_cudf/cpp_tests/utilities/table_utilities.hpp>
#include <execution_graph/logic_controllers/TaskFlowProcessor.h>

using ral::communication::messages::experimental::SampleToNodeMasterMessage;
using ral::communication::messages::experimental::ReceivedDeviceMessage;
using ral::communication::messages::experimental::ReceivedHostMessage;

using ral::communication::network::experimental::Client;
using ral::communication::network::experimental::Node;
using ral::communication::network::experimental::Server;
using Address = blazingdb::transport::experimental::Address;
using GPUMessage = blazingdb::transport::experimental::GPUMessage;

constexpr uint32_t context_token = 3465;


// TODO get GPU_MEMORY_SIZE
auto GPU_MEMORY_SIZE = 4096;


// Helper function to compare two floating-point column contents
template <typename T>
void expect_column_data_equal(std::vector<T> const& lhs,
							  cudf::column_view const& rhs) {
  EXPECT_THAT(cudf::test::to_host<T>(rhs).first, lhs);
}


void ExecMaster() {
	cuInit(0);
	// Run server
	Server::start(8000, true);

	auto sizeBuffer = GPU_MEMORY_SIZE / 4;
	blazingdb::transport::experimental::io::setPinnedBufferProvider(sizeBuffer, 1);
	Server::getInstance().registerContext(context_token);
	auto cache_machine = ral::cache::create_cache_machine(ral::cache::cache_settings{.type = ral::cache::CacheType::SIMPLE});
	// auto cache_machine = std::make_shared<ral::cache::HostCacheMachine>();

	std::string message_token = SampleToNodeMasterMessage::MessageID() + "_" + std::to_string(1);
	Server::getInstance().registerListener(context_token, message_token, 
		[cache_machine](uint32_t context_token, std::string message_token, int event_id){
			if (event_id > 0) {
				auto message = Server::getInstance().getMessage(context_token, message_token);
				auto concreteMessage = std::static_pointer_cast<ReceivedDeviceMessage>(message);
				auto host_table = concreteMessage->releaseBlazingTable();
				cache_machine->addToCache(std::move(host_table));
			}	else {
				std::cout << "LAST EVENT" << std::endl;
				cache_machine->finish();
			}
		});

	std::thread([cache_machine]() {
		auto table = cache_machine->pullFromCache();
		assert(table != nullptr);
		// auto table = ral::communication::messages::experimental::deserialize_from_cpu(host_table.get());
		std::cout << "message received\n";
		expect_column_data_equal(std::vector<int32_t>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}, table->view().column(0));
		cudf::test::strings_column_wrapper expected({"d", "e", "a", "d", "k", "d", "l", "a", "b", "c"}, {1, 0, 1, 1, 1, 1, 1, 1, 0 , 1});
		cudf::test::expect_columns_equal(table->view().column(4), expected);		
		std::this_thread::sleep_for (std::chrono::seconds(1));
	}).join();
}

void ExecWorker() {
	cuInit(0);
	// todo get GPU_MEMORY_SIZE
	auto sizeBuffer = GPU_MEMORY_SIZE / 4;
	auto nthread = 4;
	blazingdb::transport::experimental::io::setPinnedBufferProvider(sizeBuffer, nthread);
	auto sender_node = Node(Address::TCP("127.0.0.1", 8001, 1234));
	auto server_node = Node(Address::TCP("127.0.0.1", 8000, 1234));

	const auto samples = blazingdb::test::build_custom_table();
	
	std::uint64_t total_row_size = samples.num_rows();
	ral::frame::BlazingTableView table_view(samples.view(), samples.names());
	std::string message_token = SampleToNodeMasterMessage::MessageID() + "_" + std::to_string(1);
	auto message = std::make_shared<SampleToNodeMasterMessage>(message_token, context_token, sender_node, table_view, total_row_size);

	Client::send(server_node, *message);
	Client::notifyLastMessageEvent(server_node, message->metadata());
	std::this_thread::sleep_for (std::chrono::seconds(1));
}


struct SendBatchSamplesTest : public ::testing::Test {

  void SetUp() { ASSERT_EQ(rmmInitialize(nullptr), RMM_SUCCESS); }

  void TearDown() { ASSERT_EQ(rmmFinalize(), RMM_SUCCESS); }
};


// TODO: move common code of TCP client and server to blazingdb::network in order to be shared by manager and transport
// TODO: check when the ip, port is busy, return exception!
// TODO: check when the message is not registered, or the wrong message is registered
TEST_F(SendBatchSamplesTest, MasterAndWorker) {
	if(fork() > 0) {
		ExecMaster();
	} else {
		ExecWorker();
	}
}

// // TO use in separate process by:
// // ./blazingdb-communication-gtest --gtest_filter=SendBatchSamplesTest.Master
// TEST_F(SendBatchSamplesTest, Master) {
//    ExecMaster();
//  }


// //  // ./blazingdb-communication-gtest --gtest_filter=SendBatchSamplesTest.Worker
// TEST_F(SendBatchSamplesTest, Worker) {
//    ExecWorker();
//  }
