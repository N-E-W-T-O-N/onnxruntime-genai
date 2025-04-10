// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
#include "model.h"
#include "input_ids.h"
#include "logits.h"
#include "kv_cache.h"
#include "extra_inputs.h"
#include "position_inputs.h"

namespace Generators {

struct EncoderDecoderModel : Model {
  EncoderDecoderModel(std::unique_ptr<Config> config, OrtEnv& ort_env);

  std::unique_ptr<State> CreateState(DeviceSpan<int32_t> sequence_lengths, const GeneratorParams& params) const override;

  std::unique_ptr<OrtSession> session_encoder_;  // encoder_decoder_init.onnx
  std::unique_ptr<OrtSession> session_decoder_;  // decoder.onnx
};

struct EncoderDecoderState : State {
  EncoderDecoderState(const EncoderDecoderModel& model, DeviceSpan<int32_t> sequence_lengths, const GeneratorParams& params);
  EncoderDecoderState(const EncoderDecoderState&) = delete;
  EncoderDecoderState& operator=(const EncoderDecoderState&) = delete;
  DeviceSpan<float> Run(int current_length, DeviceSpan<int32_t>& next_tokens, DeviceSpan<int32_t> next_indices) override;
  void AddEncoderCrossCache(std::unique_ptr<CrossCache>& cross_cache_) {cross_cache_->AddOutputs();}
  void AddDecoderCrossCache(std::unique_ptr<CrossCache>& cross_cache_) {cross_cache_->AddInputs();}

 private:
  void UpdateInputsOutputs(DeviceSpan<int32_t>& next_tokens, DeviceSpan<int32_t> next_indices, int current_length, bool search_buffers);

  DefaultInputIDs encoder_input_ids_{*this};
  DefaultPositionInputs encoder_attention_mask_;

  // DefaultInputIDs input_ids_{*this};
  // std::unique_ptr<OrtValue> decoder_input_ids_, expanded_decoder_input_ids_;
  Logits logits_{*this};
  DefaultKeyValueCache kv_cache_{*this};

  const EncoderDecoderModel& model_;

  std::unique_ptr<CrossCache> cross_cache_;
  size_t decoder_input_ids_index_{~0U};


  // std::array<int64_t, 2> decoder_input_ids_shape_{};  // {params.batch_size*params.beam_size, params.sequence_length}
  // std::unique_ptr<Tensor> decoder_input_ids_;
  // std::unique_ptr<Tensor> decoder_input_ids_next_;  // Replaces attention_mask_ after each run
};
}  // namespace Generators
