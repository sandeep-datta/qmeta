#include "qmeta_parser.h"


bool QMetaParser::parse(int ruleId, QString inp, QVariant& ast, ParseStatusPtr& ps)
{
    return QMetaParserBase::parse(ruleId, inp, ast, ps);
}

QMetaParser::QMetaParser()
{
    initRuleMap();
}

bool QMetaParser::parse(int ruleId, int pos, QVariant &ast, ParseStatusPtr& ps)
{
    bool ok = false;
    QSTDOUT() << "Entering: " << __FUNCTION__ << "(" << pos << ")" << endl;
    g_indentLevel++;
    ok = applyRule(ruleId, pos, ast, ps);

//_exit:
    g_indentLevel--;
    QSTDOUT() << "Leaving: " << __FUNCTION__ << "() = " << ast << ", " << ok << endl;
    return ok;
}

bool QMetaParser::rules(int& pos, QVariant &ast, ParseStatusPtr& ps)
{
    bool ok = false;
    QSTDOUT() << "Entering: " << __FUNCTION__ << "(" << pos << ")" << endl;
    g_indentLevel++;
    QList<QVariant> l;

    QVariant _ast;
    while (applyRule(RULE, pos, _ast, ps)) {
        EXPECT(thisToken(pos, QSL(";"), ps));
        l.append(_ast);
    }

    ast = l;

    RETURN_SUCCESS();
_exit:
    g_indentLevel--;
    QSTDOUT() << "Leaving: " << __FUNCTION__ << "() = " << ast << ", " << ok << endl;
    return ok;
}

bool QMetaParser::grammar(int &pos, QVariant &ast, ParseStatusPtr &ps)
{
    bool ok = false;
    QSTDOUT() << "Entering: " << __FUNCTION__ << "(" << pos << ")" << endl;
    g_indentLevel++;

    QList<QVariant> l;
    l.append(QString(QSL("GRAMMAR")));

    EXPECT(thisToken(pos, "qmeta", ps));

    {
        QVariant id;
        EXPECT(applyRule(IDENTIFIER, pos, id, ps));
        l.append(id);
    }

    EXPECT(thisToken(pos, "{", ps));

    {
        QVariant _ast;
        EXPECT(applyRule(RULES, pos, _ast, ps));
        l.append(_ast);
    }

    EXPECT(thisToken(pos, "}", ps));

    ast = l;

    RETURN_SUCCESS();
_exit:
    g_indentLevel--;
    QSTDOUT() << "Leaving: " << __FUNCTION__ << "() = " << ast << ", " << ok << endl;
    return ok;
}

bool QMetaParser::rule(int &pos, QVariant &ast, ParseStatusPtr& ps)
{
    bool ok = false;
    QSTDOUT() << "Entering: " << __FUNCTION__ << "(" << pos << ")" << endl;
    g_indentLevel++;

    QList<QVariant> l;
    l.append(QString(QSL("RULE")));

    {
        QVariant id;
        EXPECT(applyRule(IDENTIFIER, pos, id, ps));
        l.append(id);
    }

    {
        EXPECT(thisToken(pos, QSL("="), ps));
    }

    {
        QVariant _ast;
        EXPECT(applyRule(CHOICES, pos, _ast, ps));
        l.append(_ast);
    }

    ast = l;

    RETURN_SUCCESS();
_exit:
    g_indentLevel--;
    QSTDOUT() << "Leaving: " << __FUNCTION__ << "() = " << ast << ", " << ok << endl;
    return ok;
}

bool QMetaParser::choices(int &pos, QVariant &ast, ParseStatusPtr& ps)
{
    bool ok = false;
    QSTDOUT() << "Entering: " << __FUNCTION__ << "(" << pos << ")" << endl;
    g_indentLevel++;

    CHECK_POINT(cp0, pos);
    {
        pos = cp0;
        QList<QVariant> l;
        l.append(QString(QSL("OR")));

        QVariant _ast;
        TRY(applyRule(CHOICE, pos, _ast, ps), choice1);
        l.append(_ast);

        TRY(thisToken(pos, QSL("|"), ps), choice1);
        TRY(applyRule(CHOICES, pos, _ast, ps), choice1);

        ast = l;
        RETURN_SUCCESS();
    }

choice1:
    {
        pos = cp0;
        QVariant _ast;
        EXPECT(applyRule(CHOICE, pos, _ast, ps));
        ast = _ast;
    }

    RETURN_SUCCESS();

_exit:
    g_indentLevel--;
    QSTDOUT() << "Leaving: " << __FUNCTION__ << "() = " << ast << ", " << ok << endl;
    return ok;
}

bool QMetaParser::choice(int &pos, QVariant &ast, ParseStatusPtr& ps)
{
    bool ok = false;
    QSTDOUT() << "Entering: " << __FUNCTION__ << "(" << pos << ")" << endl;
    g_indentLevel++;

    QList<QVariant> l;

    QVariant _ast;
    while (applyRule(TERM, pos, _ast, ps)) {
        spaces(pos, ps);
        l.append(_ast);
    }

    if(l.length() <= 0) {
        RETURN_FAILURE(pos, "choice failed");
    }

    if (thisToken(pos, QSL("->"), ps)) {
        l.append(QString(QSL("HOSTEXPR")));
        QVariant _hostExpr;
        EXPECT(applyRule(HOST_EXPR, pos, _hostExpr, ps));
        l.append(_hostExpr);
    }

    ast = l;

    RETURN_SUCCESS();

_exit:
    g_indentLevel--;
    QSTDOUT() << "Leaving: " << __FUNCTION__ << "() = " << ast << ", " << ok << endl;
    return ok;
}

bool QMetaParser::hostExpr(int &pos, QVariant &ast, ParseStatusPtr &ps)
{
    bool ok = false;
    QSTDOUT() << "Entering: " << __FUNCTION__ << "(" << pos << ")" << endl;
    g_indentLevel++;

    spaces(pos, ps);
    int count = 0;
    QChar c;
    QString hostexpr;

    while(someChar(pos, c, ps)) {
        if (c == QChar('{')) {
            count++;
            if(count == 1) {
                continue;
            }
        }
        if (c == QChar('}')) {
            count--;
            if(count == 0) {
                break;
            }
        }
        hostexpr += c;
    }

    if(count) {
        RETURN_FAILURE(pos, QSL("Invalid host expression (unbalanced braces)."));
    }

    spaces(pos, ps);

    ast = hostexpr;

    RETURN_SUCCESS();

_exit:
    g_indentLevel--;
    QSTDOUT() << "Leaving: " << __FUNCTION__ << "() = " << ast << ", " << ok << endl;
    return ok;
}

bool QMetaParser::term(int &pos, QVariant &ast, ParseStatusPtr &ps)
{
    bool ok = false;
    QSTDOUT() << "Entering: " << __FUNCTION__ << "(" << pos << ")" << endl;
    g_indentLevel++;

    CHECK_POINT(cp0, pos);
    {
        pos = cp0;
        QList<QVariant> l;
        l.append(QString(QSL("term")));

        QVariant _ast;
        TRY(applyRule(TERM1, pos, _ast, ps), choice1);
        l.append(_ast);

        TRY(thisToken(pos, QSL(":"), ps), choice1);

        QVariant id;
        TRY(applyRule(IDENTIFIER, pos, id, ps), choice1);
        l.append(id);

        ast = l;
        RETURN_SUCCESS();
    }

choice1:
    {
        pos = cp0;
        QVariant _ast;
        EXPECT(applyRule(TERM1, pos, _ast, ps));
        ast = _ast;
    }

    RETURN_SUCCESS();

_exit:
    g_indentLevel--;
    QSTDOUT() << "Leaving: " << __FUNCTION__ << "() = " << ast << ", " << ok << endl;
    return ok;
}

bool QMetaParser::term1(int &pos, QVariant &ast, ParseStatusPtr &ps)
{
    bool ok = false;
    QSTDOUT() << "Entering: " << __FUNCTION__ << "(" << pos << ")" << endl;
    g_indentLevel++;

    CHECK_POINT(cp0, pos);
    {
        pos = cp0;
        QList<QVariant> l;
        l.append(QString(QSL("NOT")));

        TRY(thisToken(pos, QSL("~"), ps), choice1);

        QVariant _ast;
        TRY(applyRule(TERM2, pos, _ast, ps), choice1);
        l.append(_ast);

        ast = l;
        RETURN_SUCCESS();
    }

choice1:
    {
        pos = cp0;
        QList<QVariant> l;
        l.append(QString(QSL("REPEAT{0,}")));

        QVariant _ast;
        TRY(applyRule(TERM2, pos, _ast, ps), choice2);
        l.append(_ast);

        TRY(thisToken(pos, QSL("*"), ps), choice2);

        ast = l;
        RETURN_SUCCESS();
    }

choice2:
    {
        pos = cp0;
        QList<QVariant> l;
        l.append(QString(QSL("REPEAT{1,}")));

        QVariant _ast;
        TRY(applyRule(TERM2, pos, _ast, ps), choice3);
        l.append(_ast);

        TRY(thisToken(pos, QSL("+"), ps), choice3);

        ast = l;
        RETURN_SUCCESS();
    }

choice3:
    {
        pos = cp0;
        QList<QVariant> l;
        l.append(QString(QSL("OPTIONAL")));

        QVariant _ast;
        TRY(applyRule(TERM2, pos, _ast, ps), choice4);
        l.append(_ast);

        TRY(thisToken(pos, QSL("?"), ps), choice4);

        ast = l;
        RETURN_SUCCESS();
    }

choice4:
    {
        pos = cp0;
        QVariant _ast;
        EXPECT(applyRule(TERM2, pos, _ast, ps));
        ast = _ast;
    }

    RETURN_SUCCESS();

_exit:
    g_indentLevel--;
    QSTDOUT() << "Leaving: " << __FUNCTION__ << "() = " << ast << ", " << ok << endl;
    return ok;
}

bool QMetaParser::term2(int &pos, QVariant &ast, ParseStatusPtr &ps)
{
    bool ok = false;
    QSTDOUT() << "Entering: " << __FUNCTION__ << "(" << pos << ")" << endl;
    g_indentLevel++;

    CHECK_POINT(cp0, pos);
    {
        pos = cp0;
        QList<QVariant> l;
        l.append(QString(QSL("CHAR")));

        TRY(thisChar(pos, QChar('\''), ps), choice1);

        CHECK_POINT(cp1, pos);
        TRY_INV(thisChar(pos, QChar('\''), ps), choice1);
        pos = cp1;

        QChar c;
        TRY(escapedChar(pos, c, ps), choice1);
        l.append(c);

        TRY(thisChar(pos, QChar('\''), ps), choice1);

        ast = l;
        RETURN_SUCCESS();
    }

choice1:
    {
        pos = cp0;
        QVariant _ast;
        TRY(applyRule(SOME_TOKEN, pos, _ast, ps), choice2);
        ast = _ast;
        RETURN_SUCCESS();
    }

choice2:
    {
        pos = cp0;
        QList<QVariant> l;
        l.append(QString(QSL("APPLY")));

        QVariant ruleName;
        TRY(applyRule(IDENTIFIER, pos, ruleName, ps), choice3);
        l.append(ruleName);

        ast = l;
        RETURN_SUCCESS();
    }

choice3:
    {
        pos = cp0;
        QList<QVariant> l;
        l.append(QString(QSL("ANYTHING")));

        QVariant val;
        TRY(thisChar(pos, QChar('.'), ps), choice4);
        l.append(val);

        ast = l;
        RETURN_SUCCESS();
    }

choice4:
    {
        pos = cp0;
        EXPECT(thisToken(pos, QSL("("), ps));

        QVariant _ast;
        EXPECT(applyRule(CHOICE, pos, _ast, ps));

        EXPECT(thisToken(pos, QSL(")"), ps));

        ast = _ast;
    }

    RETURN_SUCCESS();
_exit:
    g_indentLevel--;
    QSTDOUT() << "Leaving: " << __FUNCTION__ << "() = " << ast << ", " << ok << endl;
    return ok;
}

bool QMetaParser::someToken(int &pos, QVariant& ast, ParseStatusPtr &ps)
{
    bool ok = false;
    QSTDOUT() << "Entering: " << __FUNCTION__ << "(" << pos << ")" << endl;
    g_indentLevel++;

    spaces(pos, ps);

    QList<QVariant> l;
    l.append(QString(QSL("TOKEN")));

    EXPECT(thisChar(pos, QChar('"'), ps));

    {
        QString token;

        while (true) {
            CHECK_POINT(cp0, pos);
            if(thisChar(pos, QChar('"'), ps)) {
                pos = cp0;
                break;
            }
            pos = cp0;
            QChar c;
            EXPECT(someChar(pos, c, ps));
            token += c;
        }

        EXPECT(thisChar(pos, QChar('"'), ps));
        l.append(token);
    }

    spaces(pos, ps);

    ast = l;

    RETURN_SUCCESS();
_exit:
    g_indentLevel--;
    QSTDOUT() << "Leaving: " << __FUNCTION__ << "() = " << ast << ", " << ok << endl;
    return ok;
}

bool QMetaParser::escapedChar(int &pos, QChar &c, ParseStatusPtr &ps)
{
    bool ok = false;
    QSTDOUT() << "Entering: " << __FUNCTION__ << "(" << pos << ")" << endl;
    g_indentLevel++;

    CHECK_POINT(cp0, pos);

    QChar _c;

    {
        pos = cp0;
        TRY(thisChar(pos, QChar('\\'), ps), choice1);
        TRY(someChar(pos, _c, ps), choice1);
        c = unescape(_c);
        RETURN_SUCCESS();
    }

choice1:
    {
        pos = cp0;
        QChar _c;
        EXPECT(someChar(pos, _c, ps));
        c = _c;
    }
    RETURN_SUCCESS();

_exit:
    g_indentLevel--;
    QSTDOUT() << "Leaving: " << __FUNCTION__ << "() = \\'" << _c << "', " << ok << endl;
    return ok;
}

void QMetaParser::initRuleMap()
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
    m_rule[GRAMMAR] = reinterpret_cast<RuleFuncPtr>(&QMetaParser::grammar);
    m_rule[RULES] = reinterpret_cast<RuleFuncPtr>(&QMetaParser::rules);
    m_rule[RULE] = reinterpret_cast<RuleFuncPtr>(&QMetaParser::rule);
    m_rule[CHOICES] = reinterpret_cast<RuleFuncPtr>(&QMetaParser::choices);
    m_rule[CHOICE] = reinterpret_cast<RuleFuncPtr>(&QMetaParser::choice);
    m_rule[HOST_EXPR] = reinterpret_cast<RuleFuncPtr>(&QMetaParser::hostExpr);
    m_rule[TERM] = reinterpret_cast<RuleFuncPtr>(&QMetaParser::term);
    m_rule[TERM1] = reinterpret_cast<RuleFuncPtr>(&QMetaParser::term1);
    m_rule[TERM2] = reinterpret_cast<RuleFuncPtr>(&QMetaParser::term2);
    m_rule[SOME_TOKEN] = reinterpret_cast<RuleFuncPtr>(&QMetaParser::someToken);
#pragma GCC diagnostic pop
}
