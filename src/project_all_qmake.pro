PROJECT = project_all_qmake
TEMPLATE = subdirs

SUBDIRS = ATPool TestShell

ATPool.subdir = AdvThreadPool
TestShell.subdir = TestShell

TestShell.depends = ATPool
