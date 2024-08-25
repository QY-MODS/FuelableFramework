#pragma once
#include "Hooks.h"


constexpr std::uint32_t kSerializationVersion = 2;
constexpr std::uint32_t kDataKey = 'FUEL';


// https :  // github.com/ozooma10/OSLAroused/blob/29ac62f220fadc63c829f6933e04be429d4f96b0/src/PersistedData.cpp
template <typename T>
// BaseData is based off how powerof3's did it in Afterlife
class BaseData {
public:
    float GetData(FormID2 formId, T missing) {
        Locker locker(m_Lock);
        if (auto idx = m_Data.find(formId) != m_Data.end()) {
            return m_Data[formId];
        }
        return missing;
    }

    void SetData(FormID2 formId, T value) {
        Locker locker(m_Lock);
        m_Data[formId] = value;
    }

    virtual const char* GetType() = 0;

    virtual bool Save(SKSE::SerializationInterface* serializationInterface, std::uint32_t type, std::uint32_t version);
    virtual bool Save(SKSE::SerializationInterface* serializationInterface);
    virtual bool Load(SKSE::SerializationInterface* serializationInterface);

    void Clear();

    virtual void DumpToLog() = 0;

protected:
    std::map<FormID2, T> m_Data;

    using Lock = std::recursive_mutex;
    using Locker = std::lock_guard<Lock>;
    mutable Lock m_Lock;
};


class BaseFormFloat : public BaseData<float> {
public:
    void DumpToLog();
    const char* GetType() { return "BaseFormFloat"; }
};


template <typename T>
inline bool BaseData<T>::Save(SKSE::SerializationInterface* serializationInterface, std::uint32_t type,
                              std::uint32_t version) {
    if (!serializationInterface->OpenRecord(type, version)) {
        logger::error("Failed to open record for Data Serialization!");
        return false;
    }

    return Save(serializationInterface);
}

template <typename T>
inline bool BaseData<T>::Save(SKSE::SerializationInterface* serializationInterface) {
    assert(serializationInterface);
    Locker locker(m_Lock);

    const auto numRecords = m_Data.size();
    if (!serializationInterface->WriteRecordData(numRecords)) {
        logger::error("Failed to save {} data records", numRecords);
        return false;
    }

    for (const auto& [formId, value] : m_Data) {
        if (!serializationInterface->WriteRecordData(formId)) {
            logger::error("Failed to save data for FormID2: ({},{})", formId.outerKey, formId.innerKey);
            return false;
        }

        if (!serializationInterface->WriteRecordData(value)) {
            logger::error("Failed to save value data for form2: ({},{})", formId.outerKey, formId.innerKey);
            return false;
        }
    }
    return true;
}

template <typename T>
inline bool BaseData<T>::Load(SKSE::SerializationInterface* serializationInterface) {
    assert(serializationInterface);

    std::size_t recordDataSize;
    serializationInterface->ReadRecordData(recordDataSize);

    Locker locker(m_Lock);
    m_Data.clear();

    FormID2 formId;
    T value;

    for (auto i = 0; i < recordDataSize; i++) {
        serializationInterface->ReadRecordData(formId);
        // Ensure form still exists
        // bunu nasil yapacagiz?
        FormID2 fixedId;
        if (!serializationInterface->ResolveFormID(formId.outerKey, fixedId.outerKey) ||
            !serializationInterface->ResolveFormID(formId.innerKey, fixedId.innerKey)) {
            logger::error("Failed to resolve formID1st {} {} and Failed to resolve formID2nd {} {}"sv, formId.outerKey,
                          fixedId.outerKey, formId.innerKey, fixedId.innerKey);
            continue;
        }

        serializationInterface->ReadRecordData(value);
        m_Data[formId] = value;
    }
    return true;
}

template <typename T>
inline void BaseData<T>::Clear() {
    Locker locker(m_Lock);
    m_Data.clear();
};
