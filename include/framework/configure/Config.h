// Config.h

/** Config���ڶ�д���ò������Լ���̬��ȡ���������ò���ֵ
* ֧�ֶ����������ͣ�ֻҪParseģ��֧�ָ���������
* ���ò�����ģ����飬��ͬ��Ĳ������ֿ����ظ�ʹ��
* ֧�ַ��ʿ��ƣ��ֱ����ö�д���
* ����ָ���Զ���Ķ�ȡ���������������Զ���ķ�ʽ��Ӧ��д����
*/

#ifndef __FRAMEWORK_CONFIGURE_CONFIG_H_
#define __FRAMEWORK_CONFIGURE_CONFIG_H_

#include "framework/configure/Profile.h"
#include "framework/string/Format.h"
#include "framework/string/Parse.h"
#include "framework/system/LogicError.h"

//#include <boost/function.hpp>

namespace framework
{
    namespace configure
    {
        class Config;

        class ConfigItem
        {
        public:
            // ������ɱ�־����
            enum Flag {
                allow_set = 1, 
                allow_get = 2, 
            };

            // ���ò�����������������
            typedef boost::system::error_code (*settor_type)(
                ConfigItem &, 
                std::string const &);

            // ���ò�����ȡ����������
            typedef boost::system::error_code (*gettor_type)(
                ConfigItem const &, 
                std::string &);

            // ��������������
            typedef void (*deletor_type)(
                ConfigItem &);

        protected:
            ConfigItem(
                unsigned int flag = 0)
                : flag_(flag)
                , settor_(NULL)
                , gettor_(NULL)
            {
            }

            ConfigItem(
                unsigned int flag, 
                settor_type settor, 
                gettor_type gettor, 
                deletor_type deletor)
                : flag_(flag)
                , settor_(settor)
                , gettor_(gettor)
                , deletor_(deletor)
            {
            }

        private:
            // non copyable
            ConfigItem(
                ConfigItem const & r);

            ConfigItem & operator=(
                ConfigItem const & r);

        public:
            boost::system::error_code set(
                std::string const & str)
            {
                if (!(flag_ & allow_set))
                    return framework::system::logic_error::no_permission;
                return settor_(*this, str);
            }

            boost::system::error_code get(
                std::string & str) const
            {
                if (!(flag_ & allow_get))
                    return framework::system::logic_error::no_permission;
                return gettor_(*this, str);
            }

            void del()
            {
                deletor_(*this);
            }

            unsigned int flag() const
            {
                return flag_;
            }

        private:
            friend class Config;
            boost::system::error_code init(
                std::string const & str)
            {
                return settor_(*this, str);
            }

            unsigned int flag_;  // ������ɱ�־
            settor_type settor_; // ������
            gettor_type gettor_; // ��ȡ��
            deletor_type deletor_; // ������
        };

        class ConfigModule
            : private std::map<std::string, ConfigItem *>
        {
        public:
            ConfigModule(
                std::string const & name, 
                Config & conf)
                : name_(name)
                , conf_(conf)
            {
            }

            ~ConfigModule()
            {
                for (const_iterator ik = begin(); ik != end(); ++ik) {
                    (ik->second)->del();
                }
            }

        public:
            ConfigModule & operator()(
                std::string const & key, 
                ConfigItem * item);

            template <typename T>
            ConfigModule & operator()(
                std::string const & key, 
                T & t, 
                unsigned int flag)
            {
                (*this)(key, make_item(flag, t));
                return *this;
            }

            ConfigModule & operator()(
                std::pair<std::string const, ConfigItem *> key_item)
            {
                (*this)(key_item.first, key_item.second);
                return *this;
            }

            ConfigModule & operator<<(
                std::pair<std::string const, ConfigItem *> key_item)
            {
                (*this)(key_item.first, key_item.second);
                return *this;
            }

            boost::system::error_code set(
                std::string const & key, 
                std::string const & value)
            {
                const_iterator ik = find(key);
                if (ik == end())
                    return framework::system::logic_error::item_not_exist;
                return ik->second->set(value);
            }

            boost::system::error_code get(
                std::string const & key, 
                std::string & value) const
            {
                const_iterator ik = find(key);
                if (ik == end())
                    return framework::system::logic_error::item_not_exist;
                return ik->second->get(value);
            }

            boost::system::error_code get(
                std::map<std::string, std::string> & kvs) const
            {
                for (const_iterator ik = begin(); ik != end(); ++ik) {
                    std::string value;
                    if (!ik->second->get(value)) {
                        kvs[ik->first] = value;
                    }
                }
                return framework::system::logic_error::succeed;
            }

        private:
            std::string name_;
            Config const & conf_;
        };

        // Ĭ�ϵ�������
        template <typename T>
        struct ConfigItemT
            : public ConfigItem
        {
        public:
            ConfigItemT(
                T & t, 
                unsigned int flag)
                : ConfigItem(
                    flag, 
                    ConfigItemT::set, 
                    ConfigItemT::get, 
                    ConfigItemT::del)
                , t_(t)
            {
            }

        private:
            // ֱ�ӽ����ַ������޸Ĳ���ֵ����
            static boost::system::error_code set(
                ConfigItem & item, 
                std::string const & str)
            {
                ConfigItemT & this_item = static_cast<ConfigItemT &>(item);
                return framework::string::parse2(str, this_item.t_);
            }

            // ֱ�ӴӲ���ֵ�����ʽ������ַ���
            static boost::system::error_code get(
                ConfigItem const & item, 
                std::string & str)
            {
                ConfigItemT const & this_item = static_cast<ConfigItemT const &>(item);
                return framework::string::format2(str, this_item.t_);
            }

            static void del(
                ConfigItem & item)
            {
                ConfigItemT & this_item = static_cast<ConfigItemT &>(item);
                delete &this_item;
            }

        private:
            T & t_;
        };

        template <typename T>
        static ConfigItemT<T> * make_item(
            T & t, 
            unsigned int flag)
        {
            return new ConfigItemT<T>(t, flag);
        }

        class Config
            : private std::map<std::string, ConfigModule>
        {
        public:
            Config();

            Config(
                std::string const & file);

            ~Config(void);

        public:
            // ����ֻ���������ļ��ڴ澵��
            Profile & profile()
            {
                return pf_;
            }

        public:
            // ע��һ�����ò�������������ͬһ��ģ��
            ConfigModule & register_module(
                std::string const & module);

            // ע��һ�����ò���
            template <typename T>
            void register_param(
                std::string const & module, 
                std::string const & key, 
                T & t,
                unsigned int flag)
            {
                register_param(module, key, make_item(t, flag));
            }

            // �������ò���ֵ
            boost::system::error_code set(
                std::string const & m, 
                std::string const & k, 
                std::string const & v, 
                bool save = false);

            boost::system::error_code set_force(
                std::string const & m, 
                std::string const & k, 
                std::string const & v);

            // ��ȡ���ò���ֵ
            boost::system::error_code get(
                std::string const & m, 
                std::string const & k, 
                std::string & v);

            // ��ȡ���ò���ֵ
            boost::system::error_code get_force(
                std::string const & m, 
                std::string const & k, 
                std::string & v);

            // ��ȡһ��ģ����������ò��������Ƽ���ֵ
            boost::system::error_code get(
                std::string const & m, 
                std::map<std::string, std::string> & kvs);

            // ��ȡ�������ò��������Ƽ���ֵ
            boost::system::error_code get(
                std::map<std::string, std::map<std::string, std::string> > & mkvs);

            boost::system::error_code sync(
                std::string const & m, 
                std::string const & k);

            boost::system::error_code sync(
                std::string const & m);

            boost::system::error_code sync();

        private:
            friend class ConfigModule;

            // ע��һ�����ò���
            void register_param(
                std::string const & module, 
                std::string const & key, 
                ConfigItem * item) const;

        private:
            Profile pf_; // �����ļ��ڴ澵��
        };

        inline ConfigModule & ConfigModule::operator()(
            std::string const & key, 
            ConfigItem * item)
        {
            (*this)[key] = item;
            conf_.register_param(name_, key, item);
            return *this;
        }

    } // namespace configure
} // namespace framework

#define CONFIG_PARAM(p, flag) \
    std::make_pair( \
    #p, \
    framework::configure::make_item(p, (flag)) \
    )

#define CONFIG_PARAM_NOACC(p) \
    std::make_pair( \
    #p, \
    framework::configure::make_item(p, (0)) \
    )

#define CONFIG_PARAM_RDONLY(p) \
    std::make_pair( \
    #p, \
    framework::configure::make_item(p, (framework::configure::ConfigItem::allow_get)) \
    )

#define CONFIG_PARAM_WRONLY(p) \
    std::make_pair( \
    #p, \
    framework::configure::make_item(p, (framework::configure::ConfigItem::allow_set)) \
    )

#define CONFIG_PARAM_RDWR(p) \
    std::make_pair( \
    #p, \
    framework::configure::make_item(p, \
    (framework::configure::ConfigItem::allow_get | framework::configure::ConfigItem::allow_set)) \
    )

#define CONFIG_PARAM_NAME(name, p, flag) \
    std::make_pair( \
    name, \
    framework::configure::make_item(p, (flag)) \
    )

#define CONFIG_PARAM_NAME_NOACC(name, p) \
    std::make_pair( \
    name, \
    framework::configure::make_item(p, (0)) \
    )

#define CONFIG_PARAM_NAME_RDONLY(name, p) \
    std::make_pair( \
    name, \
    framework::configure::make_item(p, (framework::configure::ConfigItem::allow_get)) \
    )

#define CONFIG_PARAM_NAME_WRONLY(name, p) \
    std::make_pair( \
    name, \
    framework::configure::make_item(p, (framework::configure::ConfigItem::allow_set)) \
    )

#define CONFIG_PARAM_NAME_RDWR(name, p) \
    std::make_pair( \
    name, \
    framework::configure::make_item(p, \
    (framework::configure::ConfigItem::allow_get | framework::configure::ConfigItem::allow_set)) \
    )

#endif // __FRAMEWORK_CONFIGURE_CONFIG_H_
