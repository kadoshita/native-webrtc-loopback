#include <iostream>
#include <string>
#include <vector>

#ifdef WEBRTC_WIN
#include <rtc_base/win32_socket_init.h>
#endif

#include <api/create_peerconnection_factory.h>
#include <api/peer_connection_interface.h>
#include <api/audio_codecs/builtin_audio_decoder_factory.h>
#include <api/audio_codecs/builtin_audio_encoder_factory.h>
#include <api/video_codecs/builtin_video_decoder_factory.h>
#include <api/video_codecs/builtin_video_encoder_factory.h>
#include <rtc_base/ssl_adapter.h>

class CreateSDPCallback : public webrtc::CreateSessionDescriptionObserver {
 private:
  std::function<void(webrtc::SessionDescriptionInterface*)> success;
  std::function<void(const std::string&)> failure;

 public:
  CreateSDPCallback(std::function<void(webrtc::SessionDescriptionInterface*)> s,
                    std::function<void(const std::string&)> f)
      : success(s), failure(f) {}
  void OnSuccess(webrtc::SessionDescriptionInterface* desc) {
    std::cout << __LINE__ << " " << __FUNCTION__ << std::endl;
    if (success) {
      success(desc);
    }
  }
  void OnFailure(const std::string& error) {
    std::cout << __FUNCTION__ << std::endl;
    if (failure) {
      failure(error);
    } else {
      std::cout << error << std::endl;
    }
  }
};
class DummySetSessionDescriptionObserver
    : public webrtc::SetSessionDescriptionObserver {
 public:
  static DummySetSessionDescriptionObserver* Create() {
    return new rtc::RefCountedObject<DummySetSessionDescriptionObserver>();
  }
  virtual void OnSuccess() {
    std::cout << __LINE__ << " " << __FUNCTION__ << std::endl;
  }
  virtual void OnFailure(const std::string& error) {
    std::cout << __FUNCTION__ << " " << error << std::endl;
  }

 protected:
  DummySetSessionDescriptionObserver() {}
  ~DummySetSessionDescriptionObserver() {}
};
class PeerConnectionCallback : public webrtc::PeerConnectionObserver {
 private:
  std::function<void(rtc::scoped_refptr<webrtc::MediaStreamInterface>)>
      onAddStream;
  std::function<void(const webrtc::IceCandidateInterface*)> onIceCandidate;
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> pc;
  std::vector<std::string> RTCSignalingState = {"stable",
                                                "have-local-offer",
                                                "have-remote-offer",
                                                "have-local-pranswer",
                                                "have-remote-pranswer",
                                                "closed"};
  std::vector<std::string> RTCIceGatheringState = {"new", "gathering",
                                                   "complete"};
  std::vector<std::string> RTCPeerConnectionState = {
      "new", "connecting", "connected", "disconnected", "failed", "closed"};
  std::vector<std::string> RTCIceConnectionState =
  { "new",
    "checking",
    "connected",
    "completed",
    "disconnected",
    "failed",
    "closed" };

  public : PeerConnectionCallback()
      : onAddStream(nullptr),
                           onIceCandidate(nullptr) {}
  virtual ~PeerConnectionCallback() {}
  void SetOnAddStream(
      std::function<void(rtc::scoped_refptr<webrtc::MediaStreamInterface>)>
          addStream) {
    onAddStream = addStream;
  }
  void SetPeerConnection(
      rtc::scoped_refptr<webrtc::PeerConnectionInterface> p) {
    pc = p;
  }

 protected:
  void OnSignalingChange(
      webrtc::PeerConnectionInterface::SignalingState new_state) override {
    std::cout << __FUNCTION__ << " " << this->RTCSignalingState[new_state] << std::endl;
  }
  void OnDataChannel(
      rtc::scoped_refptr<webrtc::DataChannelInterface> channel) override {}
  void OnRenegotiationNeeded() override {
    std::cout << __FUNCTION__ << " !!!!!!!!!!!!!!!!" << std::endl;
  }
  void OnIceConnectionChange(
      webrtc::PeerConnectionInterface::IceConnectionState new_state) override {
    std::cout << __FUNCTION__ << " " << this->RTCIceConnectionState[new_state] << std::endl;
  }
  void OnIceGatheringChange(
      webrtc::PeerConnectionInterface::IceGatheringState new_state) override {
    std::cout << __FUNCTION__ << " " << this->RTCIceGatheringState[new_state] << std::endl;

    if (new_state == 2) {
      std::string sdp;
      pc->local_description()->ToString(&sdp);

      std::cout
          << "\n\n"
          << "3 -------------------- send server answer ---------------------"
          << std::endl;
      std::cout << sdp << std::endl;
      std::cout
          << "3 -------------------- send server answer ---------------------"
          << std::endl;
    }
  }
  void OnIceConnectionReceivingChange(bool receiving) override {
    std::cout << __FUNCTION__ << " " << receiving << std::endl;
  }
  void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override {
    std::cout << __FUNCTION__ << std::endl;
  }
  void OnRemoveStream(
      rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override {
    std::cout << __FUNCTION__ << " " << stream->id() << std::endl;
  }
  void OnAddStream(
      rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override {
    std::cout << __FUNCTION__ << " " << stream->id() << std::endl;
    onAddStream(stream);
  }
};

rtc::scoped_refptr<webrtc::PeerConnectionInterface> CreatePeerConnection(
    webrtc::PeerConnectionObserver* observer) {
  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
      peer_connection_factory = webrtc::CreatePeerConnectionFactory(
          nullptr, nullptr,
          nullptr, nullptr,
          webrtc::CreateBuiltinAudioEncoderFactory(),
          webrtc::CreateBuiltinAudioDecoderFactory(),
          webrtc::CreateBuiltinVideoEncoderFactory(),
          webrtc::CreateBuiltinVideoDecoderFactory(), nullptr,
          nullptr);
  if (!peer_connection_factory.get()) {
    std::cout << "Failed to initialize PeerConnectionFactory" << std::endl;
    return nullptr;
  }

  webrtc::PeerConnectionInterface::RTCConfiguration config;
  webrtc::PeerConnectionInterface::IceServers servers;
  webrtc::PeerConnectionInterface::IceServer ice_server;
  ice_server.uri = "stun:stun.l.google.com:19302";
  servers.push_back(ice_server);
  config.servers = servers;

  config.enable_dtls_srtp=true;
  config.sdp_semantics = webrtc::SdpSemantics::kPlanB;

  return peer_connection_factory->CreatePeerConnection(config, nullptr, nullptr,
                                                       observer);
}
int main() {
  rtc::WinsockInitializer winsock_initializer;
  rtc::InitializeSSL();

  auto peer_connection_callback = new PeerConnectionCallback();
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection =
      CreatePeerConnection(peer_connection_callback);
  peer_connection_callback->SetPeerConnection(peer_connection);

  {  // receive offer
    std::string sdp;
    {
      std::cout << "\n\n"
                << "1. input browser offer" << std::endl;
      std::string input;
      do {
        std::getline(std::cin, input);
        if (input != "") {
          sdp += input + '\n';
        }
      } while (input != "");
      std::cout << "1. input received" << std::endl;
    }

    webrtc::SdpParseError error;
    webrtc::SessionDescriptionInterface* session_description(
        webrtc::CreateSessionDescription("offer", sdp, &error));
    if (!session_description) {
      std::cout << "Can't parse received session description message. "
                << "SdpParseError was: " << error.description << std::endl;
      return 1;
    }
    std::cout << " Received session description" << std::endl;

    peer_connection_callback->SetOnAddStream(
        [&](rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {
          peer_connection->AddStream(stream);
        });
    std::cout << "set remote description start." << std::endl;
    peer_connection->SetRemoteDescription(
        DummySetSessionDescriptionObserver::Create(), session_description);
    std::cout << "set remote description end." << std::endl;
  }

  if (peer_connection->remote_description()->type() == "offer") {
    //create answer
    webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
    options.offer_to_receive_audio = webrtc::PeerConnectionInterface::
        RTCOfferAnswerOptions::kOfferToReceiveMediaTrue;
    options.offer_to_receive_video = webrtc::PeerConnectionInterface::
        RTCOfferAnswerOptions::kOfferToReceiveMediaTrue;
    peer_connection->CreateAnswer(
        new rtc::RefCountedObject<CreateSDPCallback>(
            [peer_connection](webrtc::SessionDescriptionInterface* desc) {
              std::cout << "create answer callback start." << std::endl;
              peer_connection->SetLocalDescription(
                  DummySetSessionDescriptionObserver::Create(), desc);
              std::cout << "set set local description" << std::endl;
            },
            nullptr),
        options);
  }
  (rtc::Thread::Current())->Run();
  rtc::CleanupSSL();

  return 0;
}