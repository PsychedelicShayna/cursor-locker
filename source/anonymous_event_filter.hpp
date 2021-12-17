#ifndef ANONYMOUS_EVENT_FILTER_HPP
#define ANONYMOUS_EVENT_FILTER_HPP

#include <QtCore/QObject>

#include <functional>
#include <tuple>

template<typename ...LARGS>
class AnonymousEventFilter : public QObject {
protected:
    std::function<bool(QObject*, QEvent*, LARGS...)> eventFilterFunction;
    std::tuple<LARGS...> eventFilterFunctionArgs;

public:
    virtual bool eventFilter(QObject* watched, QEvent* event) override {
        const auto& calling_arguments { std::tuple_cat(std::make_tuple(watched, event), eventFilterFunctionArgs) };
        return std::apply(eventFilterFunction, calling_arguments);
    }

    AnonymousEventFilter(const std::function<bool(QObject*, QEvent*, LARGS...)>& function, LARGS... largs)
        :
          eventFilterFunction        { function                  },
          eventFilterFunctionArgs    { std::make_tuple(largs...) }
    {

    }
};

#endif // ANONYMOUS_EVENT_FILTER_HPP
