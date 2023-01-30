#include "hoverfilter.h"

hoverFilter::hoverFilter(QObject *parent) : QObject(parent)
{

}

bool hoverFilter(QObject* obj, QEvent *event)
{
    if(obj == target && event -> type() == QEvent::Leave)
}
