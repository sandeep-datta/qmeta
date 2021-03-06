#include "QMetaParser.h"
#include "QString/macros.h"

QMetaParser::QMetaParser(int ruleId, const QString &inp)
    : QMetaQStringToQVariantTransformer(ruleId, inp)
{
    initRuleMap();
}

bool QMetaParser::parse(QVariant& ast)
{
    return QMetaQStringToQVariantTransformer::parse(ast);
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


bool QMetaParser::parse(int ruleId, int pos, QVariant &ast, ParseErrorPtr& pe)
{
    ENTRYV(pos);

    EXPECT(applyRule(ruleId, pos, ast, cpe));

    EXITV(ast);
}

bool QMetaParser::grammar(int &pos, QVariant &ast, ParseErrorPtr& pe)
{
    ENTRYV(pos);

    QList<QVariant> l;
    l.append(QString(QSL("GRAMMAR")));

    EXPECT(thisToken(pos, "qmeta", cpe));

    {
        QVariant id;
        EXPECT(applyRule(IDENTIFIER, pos, id, cpe));
        l.append(id);
    }

    EXPECT(thisToken(pos, "{", cpe));

    {
        QVariant _ast;
        EXPECT(applyRule(RULES, pos, _ast, cpe));
        l.append(_ast);
    }

    EXPECT(thisToken(pos, "}", cpe));

    ast = l;

    EXPECT(eof(pos, cpe));

    RETURN_SUCCESS();

    EXITV(ast);
}

bool QMetaParser::rules(int& pos, QVariant &ast, ParseErrorPtr& pe)
{
    ENTRYV(pos);

    QList<QVariant> l;

    QVariant _ast;

    EXPECT(applyRule(RULE, pos, _ast, cpe));

    do {
        EXPECT(thisToken(pos, QSL(";"), cpe));
        l.append(_ast);
    } while (applyRule(RULE, pos, _ast, cpe));

    if(l.length() == 1) {
        ast = l.at(0);
    } else {
        ast = l;
    }

    RETURN_SUCCESS();

    EXITV(ast);
}

bool QMetaParser::rule(int &pos, QVariant &ast, ParseErrorPtr& pe)
{
    ENTRYV(pos);

    QList<QVariant> l;
    l.append(QString(QSL("RULE")));

    {
        QVariant id;
        EXPECT(applyRule(IDENTIFIER, pos, id, cpe));
        l.append(id);
    }

    {
        EXPECT(thisToken(pos, QSL("="), cpe));
    }

    {
        QVariant _ast;
        EXPECT(applyRule(CHOICES, pos, _ast, cpe));
        l.append(_ast);
    }

    ast = l;

    RETURN_SUCCESS();

    EXITV(ast);
}

bool QMetaParser::choices(int &pos, QVariant &ast, ParseErrorPtr& pe)
{
    ENTRYV(pos);

    CHECK_POINT(cp0);
    {
        pos = cp0;
        QList<QVariant> l;
        l.append(QString(QSL("OR")));

        QVariant _ast;
        TRY(applyRule(CHOICE, pos, _ast, cpe), choice1);
        l.append(_ast);

        TRY(thisToken(pos, QSL("|"), cpe), choice1);
        TRY(applyRule(CHOICES, pos, _ast, cpe), choice1);
        l.append(_ast);

        if(l.length() == 2) {
            ast = l.at(1);
        } else {
            ast = l;
        }

        RETURN_SUCCESS();
    }

choice1:
    {
        pos = cp0;
        QVariant _ast;
        EXPECT(applyRule(CHOICE, pos, _ast, cpe));
        ast = _ast;
    }

    RETURN_SUCCESS();

    EXITV(ast);
}

bool QMetaParser::choice(int &pos, QVariant &ast, ParseErrorPtr& pe)
{
    ENTRYV(pos);

    QList<QVariant> l0;
    QList<QVariant> l;
    l.append(QString(QSL("AND")));

    QVariant _ast;
    while (applyRule(TERM, pos, _ast, cpe)) {
        spaces(pos, cpe);
        l.append(_ast);
    }

    if(l.length() <= 0) {
        RETURN_FAILURE();
    }

    if(l.length() == 2) {
        _ast = l.at(1);
    } else {
        _ast = l;
    }

    if (thisToken(pos, QSL("->"), cpe)) {
        l0.append(QString(QSL("HOST_EXPR")));
        QVariant _hostExpr;
        EXPECT(applyRule(HOST_EXPR, pos, _hostExpr, cpe));
        l0.append(_ast);
        l0.append(_hostExpr);
        ast = l0;
    } else {
        ast = _ast;
    }

    RETURN_SUCCESS();

    EXITV(ast);
}

bool QMetaParser::hostExpr(int &pos, QVariant &ast, ParseErrorPtr& pe)
{
    ENTRYV(pos);

    spaces(pos, cpe);
    int count = 0;
    QChar c;
    QString hostexpr;

    while(someChar(pos, c, cpe)) {
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
        //RETURN_FAILURE(QSL("Invalid host expression (unbalanced braces)."));
        RETURN_FAILURE();
    }

    spaces(pos, cpe);

    ast = hostexpr;

    RETURN_SUCCESS();

    EXITV(ast);
}

bool QMetaParser::term(int &pos, QVariant &ast, ParseErrorPtr& pe)
{
    ENTRYV(pos);

    CHECK_POINT(cp0);
    {
        pos = cp0;
        QList<QVariant> l;
        l.append(QString(QSL("DEFINE")));

        QVariant _ast;
        TRY(applyRule(TERM1, pos, _ast, cpe), choice1);

        TRY(thisToken(pos, QSL(":"), cpe), choice1);

        QVariant id;
        TRY(applyRule(IDENTIFIER, pos, id, cpe), choice1);

        l.append(id);

        l.append(_ast);

        ast = l;

        RETURN_SUCCESS();
    }

choice1:
    {
        pos = cp0;
        QVariant _ast;
        EXPECT(applyRule(TERM1, pos, _ast, cpe));
        ast = _ast;
    }

    RETURN_SUCCESS();

    EXITV(ast);
}

bool QMetaParser::term1(int &pos, QVariant &ast, ParseErrorPtr& pe)
{
    ENTRYV(pos);

    CHECK_POINT(cp0);
    {
        pos = cp0;
        QList<QVariant> l;
        l.append(QString(QSL("NOT")));

        TRY(thisToken(pos, QSL("~"), cpe), choice1);

        QVariant _ast;
        TRY(applyRule(TERM2, pos, _ast, cpe), choice1);
        l.append(_ast);

        ast = l;

        RETURN_SUCCESS();
    }

choice1:
    {
        pos = cp0;
        QList<QVariant> l;
        l.append(QString(QSL("ZERO_OR_MORE")));

        QVariant _ast;
        TRY(applyRule(TERM2, pos, _ast, cpe), choice2);
        l.append(_ast);

        TRY(thisToken(pos, QSL("*"), cpe), choice2);

        ast = l;

        RETURN_SUCCESS();
    }

choice2:
    {
        pos = cp0;
        QList<QVariant> l;
        l.append(QString(QSL("ONE_OR_MORE")));

        QVariant _ast;
        TRY(applyRule(TERM2, pos, _ast, cpe), choice3);
        l.append(_ast);

        TRY(thisToken(pos, QSL("+"), cpe), choice3);

        ast = l;

        RETURN_SUCCESS();
    }

choice3:
    {
        pos = cp0;
        QList<QVariant> l;
        l.append(QString(QSL("OPTIONAL")));

        QVariant _ast;
        TRY(applyRule(TERM2, pos, _ast, cpe), choice4);
        l.append(_ast);

        TRY(thisToken(pos, QSL("?"), cpe), choice4);

        ast = l;

        RETURN_SUCCESS();
    }

choice4:
    {
        pos = cp0;
        QVariant _ast;
        EXPECT(applyRule(TERM2, pos, _ast, cpe));
        ast = _ast;
    }

    RETURN_SUCCESS();

    EXITV(ast);
}

bool QMetaParser::term2(int &pos, QVariant &ast, ParseErrorPtr& pe)
{
    ENTRYV(pos);

    CHECK_POINT(cp0);
    {
        pos = cp0;
        QList<QVariant> l;
        l.append(QString(QSL("APPLY")));
        l.append(QString(QSL("char")));

        TRY(thisChar(pos, QChar('\''), cpe), choice1);

        CHECK_POINT(cp1);
        TRY_INV(thisChar(pos, QChar('\''), cpe), choice1);
        pos = cp1;

        QChar c;
        TRY(escapedChar(pos, c, cpe), choice1);
        l.append(c);

        TRY(thisChar(pos, QChar('\''), cpe), choice1);

        ast = l;

        RETURN_SUCCESS();
    }

choice1:
    {
        pos = cp0;
        QVariant _ast;
        TRY(applyRule(SOME_TOKEN, pos, _ast, cpe), choice2);
        ast = _ast;
        RETURN_SUCCESS();
    }

choice2:
    {
        pos = cp0;
        QList<QVariant> l;
        l.append(QString(QSL("APPLY")));

        QVariant ruleName;
        TRY(applyRule(IDENTIFIER, pos, ruleName, cpe), choice3);
        l.append(ruleName);

        ast = l;

        RETURN_SUCCESS();
    }

choice3:
    {
        pos = cp0;
        QList<QVariant> l;
        l.append(QString(QSL("APPLY")));
        l.append(QString(QSL("anything")));

        TRY(thisChar(pos, QChar('.'), cpe), choice4);

        ast = l;

        RETURN_SUCCESS();
    }

choice4:
    {
        pos = cp0;
        EXPECT(thisToken(pos, QSL("("), cpe));

        QVariant _ast;
        EXPECT(applyRule(CHOICES, pos, _ast, cpe));

        EXPECT(thisToken(pos, QSL(")"), cpe));

        ast = _ast;
    }

    RETURN_SUCCESS();

    EXITV(ast);
}

bool QMetaParser::someToken(int &pos, QVariant& ast, ParseErrorPtr& pe)
{
    ENTRYV(pos);

    spaces(pos, cpe);

    QList<QVariant> l;
    l.append(QString(QSL("APPLY")));
    l.append(QString(QSL("token")));

    EXPECT(thisChar(pos, QChar('"'), cpe));

    {
        QString token;

        while (true) {
            CHECK_POINT(cp0);
            if(thisChar(pos, QChar('"'), cpe)) {
                pos = cp0;
                break;
            }
            pos = cp0;
            QChar c;
            EXPECT(someChar(pos, c, cpe));
            token += c;
        }

        EXPECT(thisChar(pos, QChar('"'), cpe));
        l.append(token);
    }

    spaces(pos, cpe);

    ast = l;

    RETURN_SUCCESS();

    EXITV(ast);
}

bool QMetaParser::escapedChar(int &pos, QChar &c, ParseErrorPtr& pe)
{
    ENTRYV(pos);

    CHECK_POINT(cp0);

    QChar _c;

    {
        pos = cp0;
        TRY(thisChar(pos, QChar('\\'), cpe), choice1);
        TRY(someChar(pos, _c, cpe), choice1);
        c = unescape(_c);
        RETURN_SUCCESS();
    }

choice1:
    {
        pos = cp0;
        QChar _c;
        EXPECT(someChar(pos, _c, cpe));
        c = _c;
    }
    RETURN_SUCCESS();

    EXITV(c);
}

