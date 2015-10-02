#include "parser.h"

/*
expr    = term (( "+" | "-" ) term)*
term    = factor (( "*" | "/" ) factor)*
factor  = value | "(" expr ")"
*/

//QTextStream qStdOut()
//{
//    static QTextStream ts( stdout );
//    return ts;
//}


void Interpreter::parse(QString inp, int &result)
{
    m_inputStr = inp;
    expr(m_inputStr.midRef(0), result);
}

QStringRef Interpreter::expectStr(QStringRef inp, QString str)
{
    if (!inp.startsWith(str)) {
        throw ParseError(QSL("ERROR: Expected %1").arg(str));
    }

    return inp.mid(str.length());
}

QStringRef Interpreter::value(QStringRef inp, int &result)
{
    if(inp.isEmpty())
        return inp;

    int count = 0;

    for(; count < inp.length() && inp.at(count).isDigit(); count++) {
    }

    if (count) {
        result = int(inp.left(count).toInt());
    }

    return inp.mid(count);
}

QStringRef Interpreter::factor(QStringRef inp, int &result)
{
    if(inp.isEmpty())
        return inp;

    QStringRef inp0 = inp;
    inp = value(inp0, result);

    if(inp != inp0)
        return inp;

    inp = expectStr(inp, QSL("("));

    inp = expr(inp.mid(1), result);

    inp = expectStr(inp, QSL(")"));

    return inp;
}

bool Interpreter::advance(QStringRef &str, int count)
{
    if(str.length() < count)
        return false;

    str = str.mid(count);
    return true;
}

bool Interpreter::somechar(QStringRef &inp, QChar &c)
{
    if(inp.isEmpty())
        return false;

    c = inp.at(0);

    return advance(inp, 1);
}


QStringRef Interpreter::term(QStringRef inp, int &result)
{
    if(inp.isEmpty())
        return inp;

    inp = factor(inp, result);
    QChar lastOp;


    while(somechar(inp, lastOp))
    {
        int r;
        inp = factor(inp, r);

        if (lastOp == QSL("*")) {
            result *= r;
        } else if (lastOp == QSL("/")) {
            result /= r;
        }
    }

    return inp;
}


QStringRef Interpreter::expr(QStringRef inp, int &result)
{
    if(inp.isEmpty())
        return inp;

    inp = term(inp, result);
    QString lastOp;

    while (!inp.isEmpty() && (inp.at(0) == QSL("+") || inp.at(0) == QSL("-")))
    {
        lastOp = inp.at(0);
        inp = inp.mid(1);

        int r;
        inp = term(inp, r);

        if (lastOp == QSL("+")) {
            result += r;
        } else if (lastOp == QSL("-")) {
            result -= r;
        }
    }

    return inp;
}
