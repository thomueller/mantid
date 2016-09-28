#include "MantidLiveData/ISIS/ISISKafkaEventListener.h"
#include "MantidLiveData/ISIS/ISISKafkaEventStreamDecoder.h"
#include "MantidLiveData/Kafka/KafkaBroker.h"

#include "MantidAPI/LiveListenerFactory.h"

namespace {
Mantid::Kernel::Logger g_log("ISISKafkaEventListener");
}

namespace Mantid {
namespace LiveData {

DECLARE_LISTENER(ISISKafkaEventListener)

/// @copydoc ILiveListener::connect
bool ISISKafkaEventListener::connect(
    const Poco::Net::SocketAddress &address,
    const API::ILiveListener::ConnectionArgs &args) {
  KafkaBroker broker(address.toString());
  try {
    const std::string eventTopic(args.instrumentName + "_event_topic"),
        runInfoTopic(args.instrumentName + "_run_topic"),
        spDetInfoTopic(args.instrumentName + "_det_spec_topic");
    m_decoder = Kernel::make_unique<ISISKafkaEventStreamDecoder>(
        broker, eventTopic, runInfoTopic, spDetInfoTopic);
  } catch (std::exception &exc) {
    g_log.error() << "ISISKafkaEventListener::connect - Connection Error: "
                  << exc.what() << "\n";
    return false;
  }
  return true;
}

/// @copydoc ILiveListener::start
void ISISKafkaEventListener::start(Kernel::DateAndTime startTime) {
  UNUSED_ARG(startTime);
  m_decoder->startCapture();
}

/// @copydoc ILiveListener::extractData
boost::shared_ptr<API::Workspace> ISISKafkaEventListener::extractData() {
  assert(m_decoder);
  // The first call to extract is very early in the start live data process
  // and we may not be completely ready yet, wait upto a maximum of 5 seconds
  // to become ready
  auto checkEnd = std::chrono::system_clock::now() + std::chrono::seconds(5);
  while (!m_decoder->hasData()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if (std::chrono::system_clock::now() > checkEnd) {
      break;
    }
  }
  return m_decoder->extractData();
}

/// @copydoc ILiveListener::isConnected
bool ISISKafkaEventListener::isConnected() {
  return (m_decoder ? m_decoder->isRunning() : false);
}

/// @copydoc ILiveListener::runStatus
API::ILiveListener::RunStatus ISISKafkaEventListener::runStatus() {
  return Running;
}

/// @copydoc ILiveListener::runNumber
int ISISKafkaEventListener::runNumber() const {
  return (m_decoder ? m_decoder->runNumber() : -1);
}
}
}
