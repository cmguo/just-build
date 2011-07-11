// TextOArchive.h

#ifndef _UTIL_ARCHIVE_TEXT_O_ARCHIVE_H_
#define _UTIL_ARCHIVE_TEXT_O_ARCHIVE_H_

#include "util/archive/StreamOArchive.h"

#include <ostream>
#include <string>

namespace util
{
    namespace archive
    {

        template <
            typename _Elem = char, 
            typename _Traits = std::char_traits<_Elem>
        >
        class TextOArchive
            : public StreamOArchive<TextOArchive<_Elem, _Traits>, _Elem, _Traits>
        {
        public:
            TextOArchive(
                std::basic_ostream<_Elem, _Traits> & os)
                : StreamOArchive<TextOArchive<_Elem, _Traits>, _Elem, _Traits>(*os.rdbuf())
                , delimiter_(none)
                , os_(os)
                , local_os_(false)
            {
            }

            TextOArchive(
                std::basic_streambuf<_Elem, _Traits> & buf)
                : StreamOArchive<TextOArchive<_Elem, _Traits>, _Elem, _Traits>(buf)
                , delimiter_(none)
                , os_(*new std::basic_ostream<_Elem, _Traits>(&buf))
                , local_os_(true)
            {
            }

            ~TextOArchive()
            {
                if (local_os_)
                    delete &os_;
            }

        public:
            /// ������д����������ͱ���
            template<class T>
            void save(
                T const & t)
            {
                newtoken();
                if (this->state())
                    return;
                os_ << t;
                if(os_.fail())
                    this->state(1);
            }

            /// ������д���׼���ַ���
            void save(
                std::string const & t)
            {
                save((std::size_t)t.size());
                newtoken(); // ����һ���ո�
                this->save_binary((char const *)&t[0], t.size());
            }

            using StreamOArchive<TextOArchive>::save;

        public:
            /// �Ի��з�Ϊ�ָ�������һ������������һ�������Ȼ���Զ��ָ�Ϊ�ո�ָ���
            void newline()
            {
                delimiter_ = eol;
            }

        private:
            enum {
                none,
                eol,
                space
            } delimiter_;

            /// �ָ���һ��ʼ�ǿյģ�none����д���һ�������Ժ��Ϊ�ո�space��
            void newtoken()
            {
                if (this->state())
                    return;
                switch (delimiter_)
                {
                default:
                    this->state(1);
                    break;
                case eol:
                    if (!os_.put('\n'))
                        this->state(1);
                    // �Զ��ָ�Ϊ�ո�ָ���
                    delimiter_ = space;
                    break;
                case space:
                    if (!os_.put(' '))
                        this->state(1);
                    break;
                case none:
                    delimiter_ = space;
                    break;
                }
            }

        private:
            std::basic_ostream<_Elem, _Traits> & os_;
            bool local_os_;
        };

    }  // namespace archive
} // namespace util

#endif // _UTIL_ARCHIVE_TEXT_O_ARCHIVE_H_
