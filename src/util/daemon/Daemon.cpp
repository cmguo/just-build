// Daemon.cpp

#include "util/Util.h"
#include "util/daemon/Daemon.h"

#include <framework/logger/LoggerStreamRecord.h>
using framework::logger::Logger;

#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("Daemon", 0);

namespace util
{
    namespace daemon
    {

        Daemon::Daemon()
            : io_work_(NULL)
            , module_registry_(new detail::ModuleRegistry(*this))
        {
            //logger_.load_config(config_);
        }

        Daemon::Daemon(
            std::string const & conf)
            : io_work_(NULL)
            , config_(conf)
            , module_registry_(new detail::ModuleRegistry(*this))
        {
            //logger_.load_config(config_);
        }

        Daemon::~Daemon()
        {
            quick_stop();
            delete module_registry_;
        }

        int Daemon::parse_cmdline(
            int argc, 
            char const * argv[])
        {
            char const ** p = argv;
            for (int i = 0; i < argc; ++i) {
                if (argv[i][0] == '-' && argv[i][1] == '-') {
                    config_.profile().pre_set(argv[i] + 2);
                } else if (argv[i][0] == '+' && argv[i][1] == '+') {
                    config_.profile().post_set(argv[i] + 2);
                } else {
                    *p++ = argv[i];
                }
            }
            return p - argv;
        }

        static void startup_notify(
            boost::system::error_code & result2, 
            boost::mutex & mutex, 
            boost::condition_variable & cond, 
            boost::system::error_code const & result)
        {
            boost::mutex::scoped_lock lock(mutex);
            result2 = result;
            cond.notify_all();
        }

        static void startup_result(
            Daemon & daemon, 
            boost::system::error_code & result2, 
            boost::system::error_code const & result)
        {
            LOG_S(Logger::kLevelDebug, "[start] end");
            result2 = result;
            if (result) {
                daemon.post_stop();
            }
        }

        boost::system::error_code Daemon::start(
            size_t concurrency)
        {
            io_work_ = new boost::asio::io_service::work(io_svc_);
            boost::system::error_code result;
            LOG_S(Logger::kLevelDebug, "[start] beg");
            if (concurrency == 0) {
                io_svc_.post(boost::bind(startup_result, 
                    boost::ref(*this), 
                    boost::ref(result), 
                    boost::bind(&detail::ModuleRegistry::startup, module_registry_)));
                io_svc_.run();
                io_svc_.reset();
                LOG_S(Logger::kLevelDebug, "[stop] end");
            } else {
                boost::mutex mutex;
                boost::condition_variable cond;
                boost::mutex::scoped_lock lock(mutex);
                io_svc_.post(boost::bind(startup_notify, 
                    boost::ref(result), 
                    boost::ref(mutex), 
                    boost::ref(cond), 
                    boost::bind(&detail::ModuleRegistry::startup, module_registry_)));
                for (size_t i = 0; i < concurrency; ++i) {
                    th_grp_.create_thread(boost::bind(&boost::asio::io_service::run, &io_svc_));
                }
                cond.wait(lock);
                LOG_S(Logger::kLevelDebug, "[start] end");
                if (result) {
                    stop();
                }
            }
            return result;
        }

        static void startup_call_back(
            Daemon & daemon, 
            boost::system::error_code & result2, 
            Daemon::start_call_back_type const & start_call_back, 
            boost::system::error_code const & result)
        {
            LOG_S(Logger::kLevelDebug, "[start] end");
            result2 = result;
            start_call_back(result);
            if (result) {
                daemon.post_stop();
            }
        }

        boost::system::error_code Daemon::start(
            start_call_back_type const & start_call_back)
        {
            LOG_S(Logger::kLevelDebug, "[start] beg");
            io_work_ = new boost::asio::io_service::work(io_svc_);
            boost::system::error_code result;
            io_svc_.post(boost::bind(startup_call_back, 
                boost::ref(*this), 
                boost::ref(result), 
                boost::cref(start_call_back), 
                boost::bind(&detail::ModuleRegistry::startup, module_registry_)));
            io_svc_.run();
            io_svc_.reset();
            LOG_S(Logger::kLevelDebug, "[stop] end");
            return result;
        }

        void Daemon::stop()
        {
            LOG_S(Logger::kLevelDebug, "[stop] beg");
            delete io_work_;
            io_work_ = NULL;
            io_svc_.post(
                boost::bind(&detail::ModuleRegistry::shutdown, module_registry_));
            if (th_grp_.size()) {
                th_grp_.join_all();
                io_svc_.reset();
                LOG_S(Logger::kLevelDebug, "[stop] end");
            }
        }

        void Daemon::post_stop()
        {
            io_svc_.post(boost::bind(&Daemon::stop, this));
        }

        void Daemon::quick_stop()
        {
            if (io_work_) {
                delete io_work_;
                io_work_ = NULL;
            }
            io_svc_.stop();
            if (th_grp_.size()) {
                th_grp_.join_all();
                io_svc_.reset();
            }
        }

        bool Daemon::is_started() const
        {
            return module_registry_->is_started();
        }

    } // namespace daemon
} // namespace util
