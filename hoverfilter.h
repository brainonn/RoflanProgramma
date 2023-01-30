#ifndef HOVERFILTER_H
#define HOVERFILTER_H

#include <QObject>

class hoverFilter : public QObject
{
    Q_OBJECT
public:
    explicit hoverFilter(QObject *parent = nullptr);
    bool eventFilter(QObject* obj, QEvent *event);

signals:

public slots:
};

#endif // HOVERFILTER_H
