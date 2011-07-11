// LittleEndianBinaryOArchive.h

#ifndef _UTIL_ARCHIVE_NET_BINARY_O_ARCHIVE_H_
#define _UTIL_ARCHIVE_NET_BINARY_O_ARCHIVE_H_

#include "util/archive/StreamOArchive.h"
#include "util/serialization/Array.h"

#include <framework/system/BytesOrder.h>

#include <boost/type_traits/is_same.hpp>
#include <boost/utility/enable_if.hpp>

#include <ostream>

#ifdef BOOST_LITTLE_ENDIAN

#  include "util/archive/BinaryOArchive.h"

namespace util
{
    namespace archive
    {

        template <
            typename _Elem = char, 
            typename _Traits = std::char_traits<_Elem>
        >
        class LittleEndianBinaryOArchive
            : public BinaryOArchive<_Elem, _Traits>
        {
        public:
            LittleEndianBinaryOArchive(
                std::basic_ostream<_Elem, _Traits> & os)
                : BinaryOArchive<_Elem, _Traits>(*os.rdbuf())
            {
            }

            LittleEndianBinaryOArchive(
                std::basic_streambuf<_Elem, _Traits> & buf)
                : BinaryOArchive<_Elem, _Traits>(buf)
            {
            }
        };

    } // namespace archive
} // namespace util

#else // BOOST_LITTLE_ENDIAN

namespace util
{
    namespace archive
    {

        /**
        ת���ֽ�˳���ģ�壬����˳��->����˳��

        Ϊ���ñ������Զ�����Ӧ��ת������
        ע�⣺û��ʵ��8���ֽ����ݵ��ֽ�˳��ת��
        */ 
        template <typename T, int size = sizeof(T)>
        struct htol
        {
            static void apply(
                T &)
            {
            }
        };

        /// 2���ֽ����ݵ��ֽ�˳��ת��
        template <typename T>
        struct htol<T, 2>
        {
            static void apply(
                T & t)
            {
                t = (T)framework::system::BytesOrder::host_to_little_endian_short(t);
            }
        };

        /// 4���ֽ����ݵ��ֽ�˳��ת��
        template <typename T>
        struct htol<T, 4>
        {
            static void apply(
                T & t)
            {
                t = (T)framework::system::BytesOrder::host_to_little_endian_long(t);
            }
        };

        template <typename T>
        struct htol<T, 8>
        {
            static void apply(
                T & t)
            {
                t = (T)framework::system::BytesOrder::host_to_little_endian_longlong(t);
            }
        };

        /// �����ֽ�˳�����л���
        template <
            typename _Elem = char, 
            typename _Traits = std::char_traits<_Elem>
        >
        class LittleEndianBinaryOArchive
            : public StreamOArchive<LittleEndianBinaryOArchive<_Elem, _Traits>, _Elem, _Traits>
        {
            friend class StreamOArchive<LittleEndianBinaryOArchive<_Elem, _Traits>, _Elem, _Traits>;

        public:
            /// ��ostream����
            LittleEndianBinaryOArchive(
                std::basic_ostream<_Elem, _Traits> & os)
                : StreamOArchive<LittleEndianBinaryOArchive<_Elem, _Traits>, _Elem, _Traits>(*os.rdbuf())
            {
            }

            LittleEndianBinaryOArchive(
                std::basic_streambuf<_Elem, _Traits> & buf)
                : StreamOArchive<LittleEndianBinaryOArchive<_Elem, _Traits>, _Elem, _Traits>(buf)
            {
            }

            /// ���л�ͨ������ʵ��
            template <typename T>
            void save(
                T const & t)
            {
                // ��ת���ֽ�˳��
                T t1 = t;
                htol<T>::apply(t1);
                this->save_binary((_Elem const *)&t1, sizeof(T));
            }

            using StreamOArchive<LittleEndianBinaryOArchive<_Elem, _Traits>, _Elem, _Traits>::save;

            /// �ж�ĳ�������Ƿ�����Ż���������л�
            /// ֻ��char�����ܹ�ֱ�����л����飬����Ҫת���ֽ�˳��
            template<class T>
            struct use_array_optimization
                : boost::integral_constant<bool, sizeof(T) == 1>
            {
            };

            // ���л����飬ֱ�Ӷ���������д�룬�����char������Ż�ʵ��
            template<class T>
            void save_array(
                framework::container::Array<T> const & a, 
                typename boost::enable_if<use_array_optimization<T> >::type * = NULL)
            {
                this->save_binary((_Elem *)a.address(), a.count());
            }
        };

    } // namespace archive
} // namespace util

#endif // BOOST_LITTLE_ENDIAN

namespace util
{
    namespace serialization
    {
        template <
            typename _Elem, 
            typename _Traits, 
            typename T
        >
        struct use_array_optimization<util::archive::LittleEndianBinaryOArchive<_Elem, _Traits>, T>
            : util::archive::LittleEndianBinaryOArchive<_Elem, _Traits>::template use_array_optimization<T>
        {
        };
    }
}

#endif // _UTIL_ARCHIVE_NET_BINARY_O_ARCHIVE_H_
