#include <xoc_types.h>


int type_size(type_t* type) {
    switch (type->kind) {
        case XOC_TYPE_VOID: return 0;
        case XOC_TYPE_BOOL: return sizeof(bool);
        case XOC_TYPE_I8:   return sizeof(int8_t);
        case XOC_TYPE_U8:   return sizeof(uint8_t);
        case XOC_TYPE_I16:  return sizeof(int16_t);
        case XOC_TYPE_U16:  return sizeof(uint16_t);
        case XOC_TYPE_I32:  return sizeof(int32_t);
        case XOC_TYPE_U32:  return sizeof(uint32_t);
        case XOC_TYPE_I64:  return sizeof(int64_t);
        case XOC_TYPE_U64:  return sizeof(uint64_t);
        case XOC_TYPE_F32:  return sizeof(float);
        case XOC_TYPE_F64:  return sizeof(double);
        case XOC_TYPE_CHAR: return sizeof(unsigned char);
        case XOC_TYPE_STR:  return sizeof(char*);
        case XOC_TYPE_PTR:  return sizeof(void*);
        case XOC_TYPE_WPTR: return sizeof(uint64_t);
        case XOC_TYPE_STRUCT: 
        case XOC_TYPE_INTERFACE: 
        case XOC_TYPE_CLOSURE: {
        
        }
        default:
            return -1;
    };
}