#ifndef B9PRINT_H
#define B9PRINT_H

#include <QDialog>

namespace Ui {
class B9Print;
}

class B9Print : public QDialog
{
    Q_OBJECT
    
public:
    explicit B9Print(QWidget *parent = 0);
    ~B9Print();
    
private:
    Ui::B9Print *ui;
};

#endif // B9PRINT_H
