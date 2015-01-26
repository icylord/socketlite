#ifndef SOCKETLITE_RPC_THRIFT_BINARY_PROTOCOL_H
#define SOCKETLITE_RPC_THRIFT_BINARY_PROTOCOL_H
#ifdef SOCKETLITE_USE_THRIFT

#include <limits>
#include <string>
#include <boost/shared_ptr.hpp>
#include <protocol/TProtocol.h>
#include "SL_ByteBuffer.h"

/**
 * The default binary protocol for thrift. Writes all data in a very basic
 * binary format, essentially just spitting out the raw bytes.
 *
 */

using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;

template <typename TByteBuffer>
class SL_Rpc_Thrift_BinaryProtocol : public apache::thrift::protocol::TProtocol
{
protected:
    static const int32_t VERSION_MASK = 0xffff0000;
    static const int32_t VERSION_1 = 0x80010000;
    // VERSION_2 (0x80020000)  is taken by TDenseProtocol.

public:
    inline SL_Rpc_Thrift_BinaryProtocol()
        : write_buffer_(NULL)
        , read_buffer_(NULL)
        , read_buffer_len_(0)
    {
    }

    inline virtual ~SL_Rpc_Thrift_BinaryProtocol()
    {
    }

    inline void setBuffer(char *read_buffer, int read_buffer_len, TByteBuffer *write_buffer)
    {
        write_buffer_    = write_buffer;
        read_buffer_     = read_buffer;
        read_buffer_len_ = read_buffer_len;
    }

    /**
    * Writing functions.
    */

    inline uint32_t writeMessageBegin(const std::string &name, const TMessageType messageType, const int32_t seqid) 
    {
        int32_t  version = (VERSION_1) | ((int32_t)messageType);
        uint32_t wsize = writeI32(version);
        wsize += writeString(name);
        wsize += writeI32(seqid);
        return wsize;
    }

    inline uint32_t writeMessageEnd() 
    {
        return 0;
    }

    inline uint32_t writeStructBegin(const char* name) 
    {
        return 0;
    }

    inline uint32_t writeStructEnd() 
    {
        return 0;
    }

    inline uint32_t writeFieldBegin(const char* name, const TType fieldType, const int16_t fieldId)
    {
        uint32_t wsize = writeByte((int8_t)fieldType);
        wsize += writeI16(fieldId);
        return wsize;
    }

    inline uint32_t writeFieldEnd() 
    {
        return 0;
    }

    inline uint32_t writeFieldStop() 
    {
        return writeByte((int8_t)T_STOP);
    }

    inline uint32_t writeMapBegin(const TType keyType, const TType valType, const uint32_t size)
    {
        uint32_t wsize = writeByte((int8_t)keyType);
        wsize += writeByte((int8_t)valType);
        wsize += writeI32((int32_t)size);
        return wsize;
    }

    inline uint32_t writeMapEnd() 
    {
        return 0;
    }

    inline uint32_t writeListBegin(const TType elemType, const uint32_t size)
    {
        uint32_t wsize = writeByte((int8_t) elemType);
        wsize += writeI32((int32_t)size);
        return wsize;
    }

    inline uint32_t writeListEnd() 
    {
        return 0;
    }

    inline uint32_t writeSetBegin(const TType elemType, const uint32_t size)
    {
        uint32_t wsize = writeByte((int8_t)elemType);
        wsize += writeI32((int32_t)size);
        return wsize;
    }

    inline uint32_t writeSetEnd() 
    {
        return 0;
    }

    inline uint32_t writeBool(const bool value) 
    {
        uint8_t tmp = value ? 1 : 0;
        write_buffer_->write((const char *)&tmp, 1);
        return 1;
    }

    inline uint32_t writeByte(const int8_t byte) 
    {
        write_buffer_->write((const char *)&byte, 1);
        return 1;
    }

    inline uint32_t writeI16(const int16_t i16) 
    {
        int16_t net = (int16_t)htons(i16);
        write_buffer_->write((const char *)&net, 2);
        return 2;
    }

    inline uint32_t writeI32(const int32_t i32) 
    {
        int32_t net = (int32_t)htonl(i32);
        write_buffer_->write((const char *)&net, 4);
        return 4;
    }

    inline uint32_t writeI64(const int64_t i64) 
    {
        int64_t net = (int64_t)htonll(i64);
        write_buffer_->write((const char *)&net, 8);
        return 8;
    }

    inline uint32_t writeDouble(const double dub) 
    {
        BOOST_STATIC_ASSERT(sizeof(double) == sizeof(uint64_t));
        BOOST_STATIC_ASSERT(std::numeric_limits<double>::is_iec559);

        uint64_t bits = bitwise_cast<uint64_t>(dub);
        bits = htonll(bits);
        write_buffer_->write((const char *)&bits, 8);
        return 8;
    }

    inline uint32_t writeString(const std::string& str) 
    {
        uint32_t size   = str.size();
        uint32_t result = writeI32((int32_t)size);
        if (size > 0) 
        {
            write_buffer_->write((const char *)str.data(), size);
        }
        return result + size;
    }

    inline uint32_t writeBinary(const std::string& str) 
    {
        return writeString(str);
    }

    /**
    * Reading functions
    */

    inline uint32_t readMessageBegin(std::string& name, TMessageType& messageType, int32_t& seqid) 
    {
        int32_t  sz;
        uint32_t result = readI32(sz);
        if (sz >= 0)
        {
            // Handle pre-versioned input
            int8_t type;
            result  += readStringBody(name, sz);
            result  += readByte(type);
            result  += readI32(seqid);
            messageType = (TMessageType)type;
        }
        else
        {
            // Check for correct version number
            int32_t version = sz & VERSION_MASK;
            if (version != VERSION_1)
            {
                throw TProtocolException(TProtocolException::BAD_VERSION, "Bad version identifier");
            }
            messageType = (TMessageType)(sz & 0x000000ff);
            result += readString(name);
            result += readI32(seqid);
        }
        return result;
    }

    inline uint32_t readMessageEnd() 
    {
        return 0;
    }

    inline uint32_t readStructBegin(std::string& name) 
    {
        return 0;
    }

    inline uint32_t readStructEnd() 
    {
        return 0;
    }

    inline uint32_t readFieldBegin(std::string& name, TType& fieldType, int16_t& fieldId)
    {
        int8_t   type;
        uint32_t result = readByte(type);
        fieldType = (TType)type;
        if (fieldType == T_STOP)
        {
            fieldId = 0;
            return result;
        }
        result += readI16(fieldId);
        return result;
    }

    inline uint32_t readFieldEnd() 
    {
        return 0;
    }

    inline uint32_t readMapBegin(TType& keyType, TType& valType, uint32_t& size)
    {
        int8_t   k, v;
        int32_t  sizei;
        uint32_t result = readByte(k);
        keyType = (TType)k;
        result += readByte(v);
        valType = (TType)v;
        result += readI32(sizei);
        if (sizei >= 0)
        {
            size = (uint32_t)sizei;
            return result;
        }
        else
        {
            throw TProtocolException(TProtocolException::NEGATIVE_SIZE);
            return 0;
        }
    }

    inline uint32_t readMapEnd() 
    {
        return 0;
    }

    inline uint32_t readListBegin(TType& elemType, uint32_t& size) 
    {
        int8_t   e;
        int32_t  sizei;
        uint32_t result = readByte(e);
        elemType = (TType)e;
        result += readI32(sizei);
        if (sizei >= 0)
        {
            size = (uint32_t)sizei;
            return result;
        }
        else
        {
            throw TProtocolException(TProtocolException::NEGATIVE_SIZE);
            return 0;
        } 
    }

    inline uint32_t readListEnd() 
    {
        return 0;
    }

    inline uint32_t readSetBegin(TType& elemType, uint32_t& size)
    {
        int8_t   e;
        int32_t  sizei;
        uint32_t result = readByte(e);
        elemType = (TType)e;
        result += readI32(sizei);
        if (sizei >= 0)
        {
            size = (uint32_t)sizei;
            return result;
        }
        else
        {
            throw TProtocolException(TProtocolException::NEGATIVE_SIZE);
            return 0;
        }
    }

    inline uint32_t readSetEnd() 
    {
        return 0;
    }

    inline uint32_t readBool(bool& value) 
    {
        char *b = readBuffer(1);
        value = (*b != 0);
        return 1;
    }

    inline uint32_t readByte(int8_t& byte) 
    {
        char *b = readBuffer(1);
        byte = *b;
        return 1;
    }

    inline uint32_t readI16(int16_t& i16) 
    {
        char *b = readBuffer(2);
        i16 = (int16_t)ntohs((*(int16_t *)b));
        return 2;
    }

    inline uint32_t readI32(int32_t& i32) 
    {
        char *b = readBuffer(4);
        i32 = (int32_t)ntohl((*(int32_t *)b));
        return 4;
    }

    inline uint32_t readI64(int64_t& i64) 
    {
        char *b = readBuffer(8);
        i64 = (int64_t)ntohll((*(int64_t *)b));
        return 8;
    }

    inline uint32_t readDouble(double& dub) 
    {
        BOOST_STATIC_ASSERT(sizeof(double) == sizeof(uint64_t));
        BOOST_STATIC_ASSERT(std::numeric_limits<double>::is_iec559);

        char *b = readBuffer(8);
        uint64_t bits;
        bits = *(uint64_t *)b;
        bits = ntohll(bits);
        dub  = bitwise_cast<double>(bits);
        return 8;
    }

    inline uint32_t readString(std::string& str) 
    {
        int32_t  size;
        uint32_t result = readI32(size);
        return result + readStringBody(str, size);
    }

    inline uint32_t readBinary(std::string& str)
    {
        return SL_Rpc_Thrift_BinaryProtocol::readString(str);
    }

protected:
    inline uint32_t readStringBody(std::string& str, int32_t size) 
    {
        if (size > 0)
        {
            char *b = readBuffer(size);
            str.assign(b, size);
            return (uint32_t)size;
        }
        else if (size < 0)
        {// Catch error cases
            throw TProtocolException(TProtocolException::NEGATIVE_SIZE);
            return 0;
        }
        else
        {// size == 0, Catch empty string case
            str.clear();
            return 0;
        }
    }

    inline char* readBuffer(int len)
    {
        if (read_buffer_len_ >= len)
        {
            char *ret = read_buffer_;
            read_buffer_     += len;
            read_buffer_len_ -= len;
            return ret;
        }
        else
        {
            throw TProtocolException(TProtocolException::INVALID_DATA);
        }
        return NULL;
    }

protected:
    //д������
    TByteBuffer *write_buffer_;

    //��������
    char *read_buffer_;         //��������
    int  read_buffer_len_;      //������������
};

#endif  //#ifdef SOCKETLITE_USE_THRIFT
#endif  //#ifndef SOCKETLITE_RPC_THRIFT_BINARY_PROTOCOL_H

