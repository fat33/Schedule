#include "htmlexport.h"
#include "schedulemodel.h"
#include "timetable.h"

#include <QAbstractItemModelTester>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPdfWriter>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QTextDocument>

class TestScheduleModel : public QObject
{
    Q_OBJECT

private:
    // Every test model is wrapped by QAbstractItemModelTester, which checks
    // that all model signals and invariants follow the Qt model/view contract.
    struct Fixture
    {
        ScheduleModel model;
        QAbstractItemModelTester tester{&model,
                                        QAbstractItemModelTester::FailureReportingMode::QtTest};
    };

    static void fill(ScheduleModel &model)
    {
        QVERIFY(model.addSubject(QStringLiteral("Math")));
        QVERIFY(model.addSubject(QStringLiteral("Physics")));
        QVERIFY(model.addGroup(QStringLiteral("10A")));
        QVERIFY(model.addGroup(QStringLiteral("10B")));
        QVERIFY(model.addGroup(QStringLiteral("11A")));
        QVERIFY(model.setLesson(0, 0, QStringLiteral("Math")));
        QVERIFY(model.setLesson(1, 3, QStringLiteral("Physics")));
        QVERIFY(model.setLesson(2, 11, QStringLiteral("Math")));
    }

private slots:
    void emptyModelHasLessonRowsButNoColumns()
    {
        Fixture f;
        QCOMPARE(f.model.rowCount(), Timetable::LessonCount + 1);
        QCOMPARE(f.model.columnCount(), 0);
    }

    void addGroupRejectsEmptyAndDuplicates()
    {
        Fixture f;
        QVERIFY(f.model.addGroup(QStringLiteral("  10A ")));
        QCOMPARE(f.model.groups(), QStringList{QStringLiteral("10A")});
        QVERIFY(!f.model.addGroup(QStringLiteral("10A")));
        QVERIFY(!f.model.addGroup(QStringLiteral("   ")));
        QCOMPARE(f.model.columnCount(), 1);
    }

    void groupRowShowsNameAndIsReadOnly()
    {
        Fixture f;
        f.model.addGroup(QStringLiteral("10A"));
        const QModelIndex groupCell = f.model.index(ScheduleModel::GroupRow, 0);
        QCOMPARE(groupCell.data().toString(), QStringLiteral("10A"));
        QVERIFY(!(f.model.flags(groupCell) & Qt::ItemIsEditable));
        QVERIFY(!f.model.setData(groupCell, QStringLiteral("other")));
        QVERIFY(f.model.flags(f.model.index(1, 0)) & Qt::ItemIsEditable);
    }

    void setDataAcceptsOnlyKnownSubjects()
    {
        Fixture f;
        f.model.addGroup(QStringLiteral("10A"));
        f.model.addSubject(QStringLiteral("Math"));
        const QModelIndex cell = f.model.index(1, 0);

        QVERIFY(!f.model.setData(cell, QStringLiteral("Chemistry")));
        QCOMPARE(cell.data().toString(), QString());

        QSignalSpy spy(&f.model, &QAbstractItemModel::dataChanged);
        QVERIFY(f.model.setData(cell, QStringLiteral("Math")));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(cell.data(Qt::EditRole).toString(), QStringLiteral("Math"));

        QVERIFY(f.model.setData(cell, QString())); // clearing is allowed
        QCOMPARE(cell.data().toString(), QString());
    }

    void allowedValuesContainEmptyChoiceAndSubjects()
    {
        Fixture f;
        f.model.addGroup(QStringLiteral("10A"));
        f.model.addSubject(QStringLiteral("Math"));
        const QStringList values =
            f.model.index(1, 0).data(ScheduleModel::AllowedValuesRole).toStringList();
        QCOMPARE(values, (QStringList{QString(), QStringLiteral("Math")}));
    }

    void removeGroupShiftsFollowingColumns()
    {
        Fixture f;
        fill(f.model);
        QVERIFY(f.model.removeGroup(1));
        QCOMPARE(f.model.groups(), (QStringList{QStringLiteral("10A"), QStringLiteral("11A")}));
        QCOMPARE(f.model.lesson(0, 0), QStringLiteral("Math"));
        QCOMPARE(f.model.lesson(1, 11), QStringLiteral("Math")); // was column 2
        QVERIFY(!f.model.removeGroup(5));
    }

    void removeSubjectClearsItsLessonsOnly()
    {
        Fixture f;
        fill(f.model);
        QVERIFY(f.model.removeSubject(QStringLiteral("Math")));
        QCOMPARE(f.model.subjects(), QStringList{QStringLiteral("Physics")});
        QCOMPARE(f.model.lesson(0, 0), QString());
        QCOMPARE(f.model.lesson(2, 11), QString());
        QCOMPARE(f.model.lesson(1, 3), QStringLiteral("Physics"));
    }

    void jsonRoundTripPreservesCellPositions()
    {
        // Regression: v1 stored only non-empty cells in a flat array, so any
        // gap shifted all following lessons after reopening the file.
        Fixture f;
        fill(f.model);

        Fixture loaded;
        QString error;
        QVERIFY2(loaded.model.fromJson(f.model.toJson(), &error), qPrintable(error));
        QCOMPARE(loaded.model.groups(), f.model.groups());
        QCOMPARE(loaded.model.subjects(), f.model.subjects());
        for (int column = 0; column < f.model.columnCount(); ++column)
            for (int i = 0; i < Timetable::LessonCount; ++i)
                QCOMPARE(loaded.model.lesson(column, i), f.model.lesson(column, i));
    }

    void fromJsonRejectsBrokenInput_data()
    {
        QTest::addColumn<QByteArray>("json");
        QTest::newRow("not json") << QByteArray("{oops");
        QTest::newRow("array root") << QByteArray("[]");
        QTest::newRow("newer format") << QByteArray(R"({"version": 99})");
        QTest::newRow("duplicate group")
            << QByteArray(R"({"version":2,"groups":[{"name":"A"},{"name":"A"}]})");
    }

    void fromJsonRejectsBrokenInput()
    {
        QFETCH(QByteArray, json);
        Fixture f;
        fill(f.model);
        const QStringList groupsBefore = f.model.groups();

        QString error;
        QVERIFY(!f.model.fromJson(json, &error));
        QVERIFY(!error.isEmpty());
        QCOMPARE(f.model.groups(), groupsBefore); // model untouched on failure
    }

    void fromJsonDropsUnknownSubjects()
    {
        Fixture f;
        const QByteArray json = R"({"version":2,"subjects":["Math"],
            "groups":[{"name":"10A","lessons":["Math","Hacking",""]}]})";
        QVERIFY(f.model.fromJson(json));
        QCOMPARE(f.model.lesson(0, 0), QStringLiteral("Math"));
        QCOMPARE(f.model.lesson(0, 1), QString());
    }

    void loadsLegacyFormat()
    {
        // Row-major cells; row 0 holds group names (v1 layout).
        QJsonArray cells;
        cells << QStringLiteral("10A") << QStringLiteral("10B") << QStringLiteral("Math")
              << QStringLiteral("Physics");
        QJsonObject root;
        root.insert(QStringLiteral("Object"), QStringLiteral("Math,Physics"));
        root.insert(QStringLiteral("Group"), QStringLiteral("10A,10B"));
        root.insert(QStringLiteral("Array"), cells);

        Fixture f;
        QVERIFY(f.model.fromJson(QJsonDocument(root).toJson()));
        QCOMPARE(f.model.groups(), (QStringList{QStringLiteral("10A"), QStringLiteral("10B")}));
        QCOMPARE(f.model.lesson(0, 0), QStringLiteral("Math"));
        QCOMPARE(f.model.lesson(1, 0), QStringLiteral("Physics"));
    }

    void contentChangedIsEmittedOnEdits()
    {
        Fixture f;
        QSignalSpy spy(&f.model, &ScheduleModel::contentChanged);
        f.model.addGroup(QStringLiteral("10A"));
        f.model.addSubject(QStringLiteral("Math"));
        f.model.setLesson(0, 0, QStringLiteral("Math"));
        f.model.setLesson(0, 0, QStringLiteral("Math")); // no-op, no signal
        QCOMPARE(spy.count(), 3);
    }

    void htmlExportEscapesUserText()
    {
        Fixture f;
        f.model.addGroup(QStringLiteral("<b>10A</b>"));
        f.model.addSubject(QStringLiteral("R&D"));
        f.model.setLesson(0, 0, QStringLiteral("R&D"));

        const QString html = scheduleToHtml(f.model, QStringLiteral("Title & Co"));
        QVERIFY(html.contains(QStringLiteral("&lt;b&gt;10A&lt;/b&gt;")));
        QVERIFY(html.contains(QStringLiteral("<td>R&amp;D</td>")));
        QVERIFY(html.contains(QStringLiteral("Title &amp; Co")));
        QVERIFY(!html.contains(QStringLiteral("<b>10A")));
        QCOMPARE(html.count(QStringLiteral("rowspan")), Timetable::PairCount);
    }
    void htmlRendersToPdf()
    {
        Fixture f;
        fill(f.model);
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = qEnvironmentVariableIsSet("SCHEDULE_PDF_OUT")
                                 ? qEnvironmentVariable("SCHEDULE_PDF_OUT")
                                 : dir.filePath(QStringLiteral("schedule.pdf"));
        {
            QPdfWriter writer(path);
            writer.setPageSize(QPageSize(QPageSize::A4));
            writer.setPageOrientation(QPageLayout::Landscape);
            QTextDocument document;
            document.setHtml(scheduleToHtml(f.model, QStringLiteral("Test")));
            document.print(&writer);
        }
        QFile pdf(path);
        QVERIFY(pdf.open(QIODevice::ReadOnly));
        QVERIFY(pdf.read(5).startsWith("%PDF-"));
        QVERIFY(pdf.size() > 1000);
    }
};

QTEST_MAIN(TestScheduleModel)
#include "tst_schedulemodel.moc"
