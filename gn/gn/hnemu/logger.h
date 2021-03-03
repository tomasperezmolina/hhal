#ifndef MOD_RM_GNEMU_H_
#define MOD_RM_GNEMU_H_

namespace hhal {

    class ConsoleLogger {

        public:
            virtual ~ConsoleLogger() {};

            virtual void Trace(const char *fmt, ...);
            
            virtual void Debug(const char *fmt, ...);

            virtual void Info(const char *fmt, ...);

            virtual void Notice(const char *fmt, ...);

            virtual void Warn(const char *fmt, ...);

            virtual void Error(const char *fmt, ...);

            virtual void Crit(const char *fmt, ...);

            virtual void Alert(const char *fmt, ...);

            virtual void Fatal(const char *fmt, ...);

        };

} // namespace mango

#endif //MOD_RM_GNEMU_H_

