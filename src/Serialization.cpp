#include "Serialization.h"

void BaseFormFloat::DumpToLog() {
    Locker locker(m_Lock);
    for (const auto& [formId, value] : m_Data) {
        logger::info("Dump Row From {} - FormID1st: {} - FormID2nd: {} - value: {}", GetType(), formId.outerKey,
                     formId.innerKey, value);
    }
    // sakat olabilir
    logger::info("{} Rows Dumped For Type {}", m_Data.size(), GetType());
};