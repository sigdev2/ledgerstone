TEMPLATE        = app
TARGET          = ledgerstone

QT += widgets sql script
requires(qtConfig(treeview))
requires(qtConfig(tableview))

FORMS       =
HEADERS     = \
              QutePtr.h \
              actions.h \
              addscoredialog.h \
              calculateengine.h \
              database.h \
              defaulttheme.h \
              descriptiondialog.h \
              mainwindow.h \
              monthheadermodel.h \
              monthmodel.h \
              qutilites.h \
              scoresmodel.h \
              theme.h \
              transactionsmodel.h
RESOURCES   =
SOURCES     = \
              actions.cpp \
              addscoredialog.cpp \
              calculateengine.cpp \
              database.cpp \
              defaulttheme.cpp \
              descriptiondialog.cpp \
              mainwindow.cpp \
              monthheadermodel.cpp \
              monthmodel.cpp \
              qutilites.cpp \
              scoresmodel.cpp \
              theme.cpp \
              transactionsmodel.cpp \
              main.cpp

DISTFILES += \
    todo.txt
