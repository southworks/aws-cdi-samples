#include "Payload.h"
#include "Application.h"

Logger CdiTools::PayloadData::logger_{ "Payload" };

#ifdef TRACE_PAYLOADS
std::atomic_int CdiTools::PayloadData::next_sequence_number_ = 0;
#endif

CdiTools::PayloadData::PayloadData(uint16_t stream_identifier, size_t size)
    : CdiSgList{ 0 }
    , sgl_entry_{ 0 }
    , stream_identifier_{ stream_identifier }
    , payload_class_{ PayloadClass::Buffer }
#ifdef TRACE_PAYLOADS
    , sequence_number_{ ++next_sequence_number_ }
#endif
{
    sgl_entry_.address_ptr = Application::get()->get_pool_buffer(size);
    if (sgl_entry_.address_ptr != nullptr) {
        sgl_entry_.size_in_bytes = (int)size;
        total_data_size = (int)size;
        sgl_head_ptr = &sgl_entry_;
        sgl_tail_ptr = &sgl_entry_;
    }

#ifdef TRACE_PAYLOADS
    LOG_TRACE << "Allocated payload buffer #" << sequence_number_ << " from the pool, stream: " << stream_identifier
        << ", size: " << size << ", free items: " << Application::get()->get_pool_free_buffer_count(size) << ".";
#endif
}

CdiTools::PayloadData::PayloadData(CdiSgList sgl, uint16_t stream_identifier)
    : CdiSgList{ sgl }
    , stream_identifier_{ stream_identifier }
    , payload_class_{ PayloadClass::Sgl }
    , sgl_entry_{ sgl.sgl_head_ptr->address_ptr, sgl.sgl_head_ptr->size_in_bytes }
#ifdef TRACE_PAYLOADS
    , sequence_number_{ ++next_sequence_number_ }
#endif
{
#ifdef TRACE_PAYLOADS
    LOG_TRACE << "Constructed payload buffer #" << sequence_number_ << " from an SG list, stream: " << stream_identifier << ", size: " << size();
#endif
}

CdiTools::PayloadData::~PayloadData()
{
    if (PayloadClass::Buffer == payload_class_) {
        if (sgl_entry_.size_in_bytes > 0) {
            Application::get()->free_pool_buffer(sgl_entry_.address_ptr, sgl_entry_.size_in_bytes);
        }

#ifdef TRACE_PAYLOADS
        LOG_TRACE << "Destroyed payload buffer #" << sequence_number_ << " and released to the pool, stream: " << stream_identifier_
            << ", size: " << size()
            << ", free items: " << Application::get()->get_pool_free_buffer_count(size())
            << ".";
#endif
    }
    else if (PayloadClass::Sgl == payload_class_) {
        if (total_data_size > 0) {
            CdiCoreRxFreeBuffer(this);
        }

#ifdef TRACE_PAYLOADS
        LOG_TRACE << "Destroyed SG list payload buffer #" << sequence_number_ << ", stream: " << stream_identifier_ << ", size: " << size();
#endif
    }
}
