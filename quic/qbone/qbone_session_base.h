// Copyright (c) 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef QUICHE_QUIC_QBONE_QBONE_SESSION_BASE_H_
#define QUICHE_QUIC_QBONE_QBONE_SESSION_BASE_H_

#include "net/third_party/quiche/src/quic/core/quic_crypto_server_stream_base.h"
#include "net/third_party/quiche/src/quic/core/quic_crypto_stream.h"
#include "net/third_party/quiche/src/quic/core/quic_error_codes.h"
#include "net/third_party/quiche/src/quic/core/quic_session.h"
#include "net/third_party/quiche/src/quic/platform/api/quic_containers.h"
#include "net/third_party/quiche/src/quic/platform/api/quic_export.h"
#include "net/third_party/quiche/src/quic/qbone/qbone_packet_writer.h"
#include "net/third_party/quiche/src/common/platform/api/quiche_string_piece.h"

namespace quic {

class QUIC_EXPORT_PRIVATE QboneSessionBase : public QuicSession {
 public:
  QboneSessionBase(QuicConnection* connection,
                   Visitor* owner,
                   const QuicConfig& config,
                   const ParsedQuicVersionVector& supported_versions,
                   QbonePacketWriter* writer);
  QboneSessionBase(const QboneSessionBase&) = delete;
  QboneSessionBase& operator=(const QboneSessionBase&) = delete;
  ~QboneSessionBase() override;

  // Overrides from QuicSession.
  // This will ensure that the crypto session is created.
  void Initialize() override;
  // Called whenever a MESSAGE frame is received.
  void OnMessageReceived(quiche::QuicheStringPiece message) override;

  virtual void ProcessPacketFromNetwork(quiche::QuicheStringPiece packet) = 0;
  virtual void ProcessPacketFromPeer(quiche::QuicheStringPiece packet) = 0;

  // Returns the number of QBONE network packets that were received
  // that fit into a single QuicStreamFrame and elided the creation of
  // a QboneReadOnlyStream.
  uint64_t GetNumEphemeralPackets() const;

  // Returns the number of QBONE network packets that were received using QUIC
  // MESSAGE frame.
  uint64_t GetNumMessagePackets() const;

  void set_writer(QbonePacketWriter* writer);

 protected:
  virtual std::unique_ptr<QuicCryptoStream> CreateCryptoStream() = 0;

  // QuicSession interface implementation.
  QuicCryptoStream* GetMutableCryptoStream() override;
  const QuicCryptoStream* GetCryptoStream() const override;
  QuicStream* CreateIncomingStream(QuicStreamId id) override;
  QuicStream* CreateIncomingStream(PendingStream* pending) override;
  bool ShouldKeepConnectionAlive() const override;

  bool MaybeIncreaseLargestPeerStreamId(const QuicStreamId stream_id) override {
    return true;
  }

  // Accepts a given packet from the network and writes it out
  // to the QUIC stream. This will create an ephemeral stream per
  // packet. This function will return true if a stream was created
  // and the packet sent. It will return false if the stream could not
  // be created.
  void SendPacketToPeer(quiche::QuicheStringPiece packet);

  QbonePacketWriter* writer_;

 private:
  // Used for the crypto handshake.
  std::unique_ptr<QuicCryptoStream> crypto_stream_;

  // Statistics for the packets received by the session.
  uint64_t num_ephemeral_packets_ = 0;
  uint64_t num_message_packets_ = 0;
  uint64_t num_streamed_packets_ = 0;

  // Number of times the connection has failed to send packets as MESSAGE frame
  // and used streams as a fallback.
  uint64_t num_fallback_to_stream_ = 0;
};

}  // namespace quic

#endif  // QUICHE_QUIC_QBONE_QBONE_SESSION_BASE_H_
