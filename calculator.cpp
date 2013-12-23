#include <QApplication>
#include <QLineEdit>
#include <QPushButton>
#include <QSizePolicy>
#include <QString>

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "FRP.h"
#include "Maybe.h"

using namespace glaebhoerl;



struct Button: QPushButton
{
    Button(QString text): QPushButton(text) { }
    Events<> clicked() { return m_clicked; }

protected:
    void mouseReleaseEvent(QMouseEvent* me) override
    {
        QPushButton::mouseReleaseEvent(me);
        m_clicked();
    }

private:
    EventGen<> m_clicked;
};

using Num   = int;
using BinOp = Num (*)(Num, Num);

Events<Num> setupNumberButtons(QGridLayout* layout)
{
    std::vector<Events<Num>> buttonsClicked;

    for (Num i = 0; i <= 9; i++) {
        Button* button = new Button(QString::number(i));
        if (i == 0) {
            layout->addWidget(button, 3, 0);
        } else {
            layout->addWidget(button, 2 - ((i - 1) / 3), (i - 1) % 3);
        }
        buttonsClicked.push_back(button->clicked().taggedWith(i));
    }

    return mergedEvents(buttonsClicked);
}

Events<BinOp> setupOpButtons(QBoxLayout* layout)
{
    static const struct {
        const char* name;
        BinOp op;
    } ops[4] = {
        { "+", [] (Num a, Num b) { return a + b; } },
        { "-", [] (Num a, Num b) { return a - b; } },
        { "*", [] (Num a, Num b) { return a * b; } },
        { "/", [] (Num a, Num b) { return a / b; } }
    };

    std::vector<Events<BinOp>> buttonsClicked;

    for (int i = 0; i < 4; i++) {
        Button* button = new Button(ops[i].name);
        layout->addWidget(button);
        buttonsClicked.push_back(button->clicked().taggedWith(ops[i].op));
    }

    return mergedEvents(buttonsClicked);
}

template<typename T>
std::pair<Events<>, Events<T>> partitionedMaybes(const Events<Maybe<T>>& events)
{
    std::array<Events<Maybe<T>>, 2> asdf = events.partitionedBy(&Maybe<T>::hasValue);
    return std::make_pair(asdf[false].stripped(), *asdf[true]);
}

Variable<QString> displayedValue(Events<Num> numberClicked, Events<BinOp> opClicked, Events<> equalsClicked)
{
    BinOp overwrite = [] (Num a, Num b) { return b; };

    Events<BinOp> opOrEqualsClicked = opClicked.mergedWith(equalsClicked.taggedWith(overwrite));

    Variable<BinOp> op = opOrEqualsClicked.lastValueStartingWith(overwrite);

    using MaybeNumFunc = std::function<Maybe<Num> (Maybe<Num>)>;

    Events<MaybeNumFunc> appendDigit = numberClicked.transformedBy([] (Num n) {
        return MaybeNumFunc([n] (Maybe<Num> num) {
            return value(num.orOtherwise(0) * 10 + n);
        });
    });

    Variable<Maybe<Num>> rhs = appendDigit.mergedWith(opOrEqualsClicked.stripped().taggedWith(MaybeNumFunc([] (Maybe<Num>) { return empty<Num>(); } )))
                               .evolvingFrom(empty<Num>());

    std::pair<Events<>, Events<Num>> asdf = partitionedMaybes(rhs.changes());
    const Events<>&    rhsReset   = asdf.first;
    const Events<Num>& rhsNumbers = asdf.second;

    using NumFunc = std::function<Num (Num)>;

    Variable<NumFunc> opRhs = rhsNumbers.withSnapshotOf(op).transformedBy([] (Num rhs, BinOp op) {
        return NumFunc([rhs, op] (Num lhs) { return op(lhs, rhs); });
    }).lastValueStartingWith(NumFunc([] (Num n) { return n; }));

    Variable<Num> lhs = rhsReset.withSnapshotOf(opRhs).steppingFrom(0, [] (Num lhs, NumFunc opRhs) {
        return opRhs(lhs);
    });

    Variable<Num> displayedNum = transformedBy([] (Num lhs, Maybe<Num> rhs) { return rhs.orOtherwise(lhs); }, lhs, rhs);

    return displayedNum.transformedBy([] (Num n) { return QString::number(n); });
}

using Action = std::function<void ()>;

Variable<Action> setupUI()
{
    QWidget* ui = new QWidget;
    ui->setWindowTitle("Calculator");
    ui->show();

    QBoxLayout* l1 = new QVBoxLayout;
    ui->setLayout(l1);
    l1->setSpacing(0);
    l1->setSizeConstraint(QLayout::SetFixedSize);

    QLineEdit* output = new QLineEdit;
    output->setAlignment(Qt::AlignRight);
    output->setReadOnly(true);
    l1->addWidget(output);

    QBoxLayout* l2 = new QHBoxLayout;
    l1->addLayout(l2);
    l2->setSpacing(10);

    QBoxLayout* opsLayout = new QVBoxLayout;
    l2->addLayout(opsLayout);
    opsLayout->setSpacing(0);
    Events<BinOp> opClicked = setupOpButtons(opsLayout);

    QGridLayout* numbersLayout = new QGridLayout;
    l2->addLayout(numbersLayout);
    numbersLayout->setSpacing(0);
    Events<Num> numberClicked = setupNumberButtons(numbersLayout);

    Button* equalsButton = new Button("=");
    numbersLayout->addWidget(equalsButton, 3, 1, 1, 2);
    Events<> equalsClicked = equalsButton->clicked();

    Variable<QString> displayed = displayedValue(numberClicked, opClicked, equalsClicked);

    return displayed.transformedBy([output] (QString text) {
        return Action([output, text] { output->setText(text); });
    });
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    Variable<Action> updates = setupUI();
    updates.exec();
    return app.exec();
}
