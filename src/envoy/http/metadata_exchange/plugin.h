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

#ifndef NULL_PLUGIN

#include <assert.h>
#define ASSERT(_X) assert(_X)

#include "proxy_wasm_intrinsics.h"

static const std::string EMPTY_STRING;

#else

#include "extensions/common/wasm/null/null_plugin.h"

namespace Envoy {
namespace Extensions {
namespace Wasm {
namespace MetadataExchange {
namespace Plugin {

using namespace Envoy::Extensions::Common::Wasm::Null::Plugin;

// TODO(jplevyak): move these into the base envoy repo
using MetadataType = Envoy::Extensions::Common::Wasm::MetadataType;
using WasmResult = Envoy::Extensions::Common::Wasm::WasmResult;
using NullPluginRootRegistry =
    ::Envoy::Extensions::Common::Wasm::Null::NullPluginRootRegistry;

#endif

constexpr StringView ExchangeMetadataHeader = "x-envoy-peer-metadata";
constexpr StringView ExchangeMetadataHeaderId = "x-envoy-peer-metadata-id";

constexpr StringView NodeMetadataExchangeKeys = "EXCHANGE_KEYS";
constexpr StringView NodeIdKey = "id";
constexpr StringView WholeNodeKey = ".";

// DownstreamMetadataKey is the key in the request metadata for downstream peer
// metadata
constexpr StringView DownstreamMetadataKey =
    "envoy.wasm.metadata_exchange.downstream";
constexpr StringView DownstreamMetadataIdKey =
    "envoy.wasm.metadata_exchange.downstream_id";

// UpstreamMetadataKey is the key in the request metadata for downstream peer
// metadata
constexpr StringView UpstreamMetadataKey =
    "envoy.wasm.metadata_exchange.upstream";
constexpr StringView UpstreamMetadataIdKey =
    "envoy.wasm.metadata_exchange.upstream_id";

// PluginRootContext is the root context for all streams processed by the
// thread. It has the same lifetime as the worker thread and acts as target for
// interactions that outlives individual stream, e.g. timer, async calls.
class PluginRootContext : public RootContext {
 public:
  PluginRootContext(uint32_t id, StringView root_id)
      : RootContext(id, root_id) {}
  ~PluginRootContext() = default;

  void onConfigure(std::unique_ptr<WasmData>) override;
  void onStart(std::unique_ptr<WasmData>) override{};
  void onTick() override{};

  StringView metadataValue() { return metadata_value_; };
  StringView nodeId() { return node_id_; };

 private:
  void updateMetadataValue();
  std::string metadata_value_;
  std::string node_id_;
};

// Per-stream context.
class PluginContext : public Context {
 public:
  explicit PluginContext(uint32_t id, RootContext* root) : Context(id, root) {}

  void onCreate() override{};
  FilterHeadersStatus onRequestHeaders() override;
  FilterHeadersStatus onResponseHeaders() override;

 private:
  inline PluginRootContext* rootContext() {
    return dynamic_cast<PluginRootContext*>(this->root());
  };
  inline StringView metadataValue() { return rootContext()->metadataValue(); };
  inline StringView nodeId() { return rootContext()->nodeId(); }
};

// TODO(mjog) move this to proxy_wasm_impl.h
inline void setMetadataStruct(MetadataType type, StringView key,
                              StringView value) {
  proxy_setMetadataStruct(type, key.data(), key.size(), value.data(),
                          value.size());
}

#ifdef NULL_PLUGIN
}  // namespace Plugin
}  // namespace MetadataExchange
}  // namespace Wasm
}  // namespace Extensions
}  // namespace Envoy
#endif
