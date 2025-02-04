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

#include "extensions/common/context.h"
#include "extensions/stackdriver/config/v1alpha1/stackdriver_plugin_config.pb.h"
#include "extensions/stackdriver/metric/record.h"

// OpenCensus is full of unused parameters in metric_service.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "opencensus/exporters/stats/stackdriver/stackdriver_exporter.h"
#pragma GCC diagnostic pop

#ifndef NULL_PLUGIN
#include "api/wasm/cpp/proxy_wasm_intrinsics.h"
#else

#include "extensions/common/wasm/null/null_plugin.h"

namespace Envoy {
namespace Extensions {
namespace Common {
namespace Wasm {
namespace Null {
namespace Plugin {
#endif

namespace Stackdriver {

#ifdef NULL_PLUGIN
NULL_PLUGIN_ROOT_REGISTRY;
#endif

// StackdriverRootContext is the root context for all streams processed by the
// thread. It has the same lifetime as the worker thread and acts as target for
// interactions that outlives individual stream, e.g. timer, async calls.
class StackdriverRootContext : public RootContext {
 public:
  StackdriverRootContext(uint32_t id, StringView root_id)
      : RootContext(id, root_id) {}
  ~StackdriverRootContext() = default;

  void onConfigure(std::unique_ptr<WasmData> configuration) override;
  void onStart(std::unique_ptr<WasmData>) override;
  void onTick() override;

  // Get direction of traffic relative to this proxy.
  bool isOutbound();

  // Records telemetry based on the given request info.
  void record(const ::Wasm::Common::RequestInfo& request_info,
              const ::wasm::common::NodeInfo& peer_node_info);

 private:
  // Config for Stackdriver plugin.
  stackdriver::config::v1alpha1::PluginConfig config_;

  // Local node info extracted from node metadata.
  wasm::common::NodeInfo local_node_info_;

  // Indicates the traffic direction relative to this proxy.
  PluginDirection direction_ = PluginDirection::Unspecified;
};

// StackdriverContext is per stream context. It has the same lifetime as
// the request stream itself.
class StackdriverContext : public Context {
 public:
  StackdriverContext(uint32_t id, RootContext* root) : Context(id, root) {}
  void onLog() override;

  // Stream filter callbacks.
  FilterHeadersStatus onRequestHeaders() override;
  FilterDataStatus onRequestBody(size_t body_buffer_length,
                                 bool end_of_stream) override;
  FilterDataStatus onResponseBody(size_t body_buffer_length,
                                  bool end_of_stream) override;

 private:
  // Request information collected from stream callbacks, used when record
  // metrics and access logs.
  ::Wasm::Common::RequestInfo request_info_;

  // Peer node information extracted from peer node metadata header.
  ::wasm::common::NodeInfo peer_node_info_;

  // Gets root Stackdriver context that this stream Stackdriver context
  // associated with.
  StackdriverRootContext* getRootContext();
};

static RegisterContextFactory register_StackdriverContext(
    CONTEXT_FACTORY(StackdriverContext), ROOT_FACTORY(StackdriverRootContext));

}  // namespace Stackdriver

#ifdef NULL_PLUGIN
}  // namespace Plugin
}  // namespace Null
}  // namespace Wasm
}  // namespace Common
}  // namespace Extensions
}  // namespace Envoy
#endif
