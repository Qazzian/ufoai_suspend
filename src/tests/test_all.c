/**
 * @file test_all.c
 */

/*
Copyright (C) 2002-2010 UFO: Alien Invasion.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "stdlib.h"
#include "stdio.h"
#include <CUnit/Basic.h>
#include <CUnit/Automated.h>
#include <CUnit/Console.h>
#include <CUnit/TestDB.h>

#include "test_shared.h"

typedef int (*testSuite_t) (void);

/* include the tests here */
#include "test_generic.h"
#include "test_ui.h"
#include "test_ui_level2.h"
#include "test_routing.h"
#include "test_inventory.h"
#include "test_campaign.h"
#include "test_rma.h"
#include "test_parser.h"
#include "test_game.h"
#include "test_mapdef.h"

static const testSuite_t testSuites[] = {
	UFO_AddGenericTests,
	UFO_AddParserTests,
	UFO_AddUITests,
	UFO_AddUILevel2Tests,
	UFO_AddCampaignTests,
	UFO_AddRoutingTests,
	UFO_AddInventoryTests,
	UFO_AddRandomMapAssemblyTests,
	UFO_AddGameTests,
	UFO_AddMapDefTests,
	NULL
};
#define NUMBER_OF_TESTS (sizeof(testSuites) / sizeof(*(testSuites)))

static inline int fatalError (void)
{
	CU_cleanup_registry();
	return CU_get_error();
}

void Sys_Init(void);
void Sys_Init (void)
{
}

typedef struct config_s {
	qboolean console;
	qboolean automated;
	qboolean log;
} config_t;

static const char* resultPrefix = "ufoai";

static config_t config;

static FILE *logFile;

static CU_pSuite disabledSuites;

static qboolean displayStatus;

static void Test_List (qboolean displayStatus)
{
	CU_pTestRegistry registry;
	CU_pSuite suite;

	printf("Suites:\n");
	registry = CU_get_registry();
	suite = registry->pSuite;
	while (suite) {
		if (displayStatus)
			printf("* %s [enabled]\n", suite->pName);
		else
			printf("* %s\n", suite->pName);
		suite = suite->pNext;
	}
	suite = disabledSuites;
	while (suite) {
		if (displayStatus)
			printf("* %s [disabled]\n", suite->pName);
		else
			printf("* %s\n", suite->pName);
		suite = suite->pNext;
	}
}

static void Test_AddSuiteTo (CU_pSuite *list, CU_pSuite suite)
{
	assert(suite->pNext == NULL);
	assert(suite->pPrev == NULL);
	suite->pNext = *list;
	if (*list == NULL)
		*list = suite;
	else
		(*list)->pPrev = suite;
}

static void Test_RemoveSuiteFrom (CU_pSuite *list, CU_pSuite suite)
{
	if (*list == suite)
		*list = suite->pNext;
	if (suite->pNext)
		suite->pNext->pPrev = suite->pPrev;
	if (suite->pPrev)
		suite->pPrev->pNext = suite->pNext;
	suite->pPrev = NULL;
	suite->pNext = NULL;
}

static CU_pSuite Test_GetSuiteByName (CU_pSuite first, const char *name)
{
	CU_pSuite current = first;
	while (current) {
		if (Q_streq(current->pName, name))
			return current;
		current = current->pNext;
	}
	return NULL;
}

static int Test_RemoveSuite (const char *name)
{
	CU_pTestRegistry registry;
	CU_pSuite suite;

	registry = CU_get_registry();
	suite = Test_GetSuiteByName(registry->pSuite, name);
	if (suite) {
		Test_RemoveSuiteFrom(&registry->pSuite, suite);
		Test_AddSuiteTo(&disabledSuites, suite);
		return 0;
	}
	return 1;
}

static int Test_AddSuite (const char *name)
{
	CU_pTestRegistry registry;
	CU_pSuite suite;

	registry = CU_get_registry();
	suite = Test_GetSuiteByName(disabledSuites, name);
	if (suite) {
		Test_RemoveSuiteFrom(&disabledSuites, suite);
		Test_AddSuiteTo(&registry->pSuite, suite);
		return 0;
	}
	return 1;
}

static void Test_RemoveAllSuites (void)
{
	CU_pTestRegistry registry;
	CU_pSuite suite;

	registry = CU_get_registry();
	suite = registry->pSuite;
	while (suite) {
		CU_pSuite tmp = suite->pNext;
		Test_RemoveSuite(suite->pName);
		suite = tmp;
	}
}

static void Test_AddAllSuites (void)
{
	CU_pSuite suite;

	suite = disabledSuites;
	while (suite) {
		CU_pSuite tmp = suite->pNext;
		Test_AddSuite(suite->pName);
		suite = tmp;
	}
}

static void Test_Parameters (const int argc, const char **argv)
{
	int i;

	for (i = 1; i < argc; i++) {
		if (Q_streq(argv[i], "-c") || Q_streq(argv[i], "--console")) {
			config.console = qtrue;
		} else if (Q_streq(argv[i], "-a") || Q_streq(argv[i], "--automated")) {
			config.automated = qtrue;
		} else if (Q_streq(argv[i], "--disable-all")) {
			Test_RemoveAllSuites();
		} else if (Q_strstart(argv[i], "--disable-")) {
			const char *name = argv[i] + 10;
			if (Test_RemoveSuite(name) != 0) {
				printf("Suite \"%s\" unknown\n", name);
				printf("Use \"%s -l\" to show the list of suites\n", argv[0]);
				exit(2);
			}
		} else if (Q_streq(argv[i], "--enable-all")) {
			Test_AddAllSuites();
		} else if (Q_strstart(argv[i], "--enable-")) {
			const char *name = argv[i] + 9;
			if (Test_AddSuite(name) != 0) {
				printf("Suite \"%s\" unknown\n", name);
				printf("Use \"%s -l\" to show the list of suites\n", argv[0]);
				exit(2);
			}
		} else if (Q_strstart(argv[i], "--only-")) {
			const char *name = argv[i] + 7;
			Test_RemoveAllSuites();
			if (Test_AddSuite(name) != 0) {
				printf("Suite \"%s\" unknown\n", name);
				printf("Use \"%s -l\" to show the list of suites\n", argv[0]);
				exit(2);
			}
		} else if (Q_strstart(argv[i], "--output-prefix=")) {
			resultPrefix = argv[i] + 16;
		} else if (Q_streq(argv[i], "-l") || Q_streq(argv[i], "--list")) {
			Test_List(qfalse);
			exit(0);
		} else if (Q_streq(argv[i], "--log")) {
			config.log = qtrue;
		} else if (Q_streq(argv[i], "-s") || Q_streq(argv[i], "--status")) {
			displayStatus = qtrue;
		} else if (Q_streq(argv[i], "-h") || Q_streq(argv[i], "--help")) {
			printf("Usage:\n");
			printf("-h  --help                 | show this help screen\n");
			printf("-c  --console              | run tests in console mode\n");
			printf("-a  --automated            | run tests in automated mode (create xml file)\n");
			printf("-l  --list                 | list suite name available, and exit.\n");
			printf("-s  --status               | list suite name with there status, and exit.\n");
			printf("    --log                  | log ufo output to file (testall.log in working dir)\n");
			printf("    --output-prefix=PREFIX | set a prefix for the xml result\n");
			printf("                           | default value is \"ufoai\"\n");
			printf("    --enable-SUITE         | Enable a suite by name\n");
			printf("                           | SUITE=suite name\n");
			printf("    --enable-all           | Enable all suites\n");
			printf("    --disable-SUITE        | Disable a suite by name\n");
			printf("                           | SUITE=suite name\n");
			printf("    --disable-all          | Disable all suites\n");
			printf("    --only-SUITE           | Enable only the suite given by name\n");
			printf("                           | SUITE=suite name\n");
			exit(0);
		} else {
			printf("Param \"%s\" unknown\n", argv[i]);
			printf("Use \"%s -h\" to show the help screen\n", argv[0]);
			exit(2);
		}
	}
}

static void TEST_vPrintfLog (const char *fmt, va_list ap)
{
	if (logFile) {
		char msg[2048];

		Q_vsnprintf(msg, sizeof(msg), fmt, ap);

		fprintf(logFile, "%s", msg);
		fflush(logFile);
	}
}

/**
 * Setting up and running all tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */
int main (int argc, const char **argv)
{
	int i;
	int failures;

	Sys_InitSignals();

	/* initialize the CUnit test registry */
	if (CU_initialize_registry() != CUE_SUCCESS)
		return CU_get_error();

	for (i = 0; i < NUMBER_OF_TESTS - 1; i++) {
		if (testSuites[i]() != CUE_SUCCESS)
			return fatalError();
	}

	Test_Parameters(argc, argv);

	if (displayStatus) {
		Test_List(qtrue);
		exit(0);
	}

	if (config.log) {
		const char *path = "testall.log";
		logFile = fopen(path, "wb");
		Qcommon_SetPrintFunction(TEST_vPrintfLog);
	} else
		Qcommon_SetPrintFunction(TEST_vPrintf);

	if (config.console)
		/* Run all tests using the console interface */
		CU_console_run_tests();
	else if (config.automated) {
		CU_basic_set_mode(CU_BRM_VERBOSE);
		CU_set_output_filename(resultPrefix);
		CU_automated_run_tests();
	} else {
		/* Run all tests using the CUnit Basic interface */
		CU_basic_set_mode(CU_BRM_VERBOSE);
		CU_basic_run_tests();
	}

	failures = CU_get_number_of_failures();
	CU_cleanup_registry();

	if (logFile)
		fclose(logFile);

	/* there is a problem on the framework (use git bisect value for IDK) */
	if (CU_get_error() != 0)
		return 125;

	return failures != 0;
}
