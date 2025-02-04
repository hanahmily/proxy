/* Copyright 2019 Istio Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <set>
#include "absl/strings/string_view.h"
#include "extensions/common/node_info.pb.h"
#include "google/protobuf/struct.pb.h"

namespace Wasm {
namespace Common {

using StringView = absl::string_view;

// Node metadata
constexpr StringView WholeNodeKey = ".";

constexpr StringView kUpstreamMetadataIdKey =
    "envoy.wasm.metadata_exchange.upstream_id";
constexpr StringView kUpstreamMetadataKey =
    "envoy.wasm.metadata_exchange.upstream";

constexpr StringView kDownstreamMetadataIdKey =
    "envoy.wasm.metadata_exchange.downstream_id";
constexpr StringView kDownstreamMetadataKey =
    "envoy.wasm.metadata_exchange.downstream";

// Header keys
constexpr StringView kAuthorityHeaderKey = ":authority";
constexpr StringView kMethodHeaderKey = ":method";
constexpr StringView kContentTypeHeaderKey = "content-type";

const std::string kProtocolHTTP = "http";
const std::string kProtocolGRPC = "grpc";

const std::set<std::string> kGrpcContentTypes{
    "application/grpc", "application/grpc+proto", "application/grpc+json"};

// RequestInfo represents the information collected from filter stream
// callbacks. This is used to fill metrics and logs.
struct RequestInfo {
  // Start timestamp in nanoseconds.
  int64_t start_timestamp = 0;

  // End timestamp in nanoseconds.
  int64_t end_timestamp = 0;

  // Request total size in bytes, include header, body, and trailer.
  int64_t request_size = 0;

  // Response total size in bytes, include header, body, and trailer.
  int64_t response_size = 0;

  // Destination port that the request targets.
  uint32_t destination_port = 0;

  // Protocol used the request (HTTP/1.1, gRPC, etc).
  std::string request_protocol;

  // Response code of the request.
  uint32_t response_code = 0;

  // Response flag giving additional information - NR, UAEX etc.
  // TODO populate
  std::string response_flag;

  // Host name of destination service.
  std::string destination_service_host;

  // Operation of the request, i.e. HTTP method or gRPC API method.
  std::string request_operation;

  // Indicates if the request uses mTLS.
  bool mTLS = false;

  // Principal of source and destination workload extracted from TLS
  // certificate.
  std::string source_principal;
  std::string destination_principal;
};

// RequestContext contains all the information available in the request.
// Some or all part may be populated depending on need.
struct RequestContext {
  const bool outbound;
  const wasm::common::NodeInfo& source;
  const wasm::common::NodeInfo& destination;
  const Common::RequestInfo& request;
};

// Extracts NodeInfo from proxy node metadata passed in as a protobuf struct.
// It converts the metadata struct to a JSON struct and parse NodeInfo proto
// from that JSON struct.
// Returns status of protocol/JSON operations.
google::protobuf::util::Status extractNodeMetadata(
    const google::protobuf::Struct& metadata,
    wasm::common::NodeInfo* node_info);

// Read from local node metadata and populate node_info.
google::protobuf::util::Status extractLocalNodeMetadata(
    wasm::common::NodeInfo* node_info);

// populateHTTPRequestInfo populates the RequestInfo struct. It needs access to
// the request context.
void populateHTTPRequestInfo(bool outbound, RequestInfo* request_info);

}  // namespace Common
}  // namespace Wasm
