#define SIZEOF_REVGRAPH	19	/* Size of revision ancestry graphics. */
	REQ_(TOGGLE_LINENO,	"Toggle line numbers"), \
	REQ_(TOGGLE_REV_GRAPH,	"Toggle revision graph visualization"),
static bool opt_rev_graph	= TRUE;
		die("unknown option '%s'\n\n%s", opt, usage);
/*
 * Line-oriented content detection.
 */

		unsigned int view_lines = view->offset + view->height;
				   ? MIN(view_lines, view->lines) * 100 / view->lines
	case REQ_TOGGLE_REV_GRAPH:
		opt_rev_graph = !opt_rev_graph;
		redraw_display();
		break;

	char id[41];			/* SHA1 ID. */
	char title[75];			/* First line of the commit message. */
	char author[75];		/* Author of the commit. */
	struct tm time;			/* Date from the author ident. */
	struct ref **refs;		/* Repository references. */
	chtype graph[SIZEOF_REVGRAPH];	/* Ancestry chain graphics. */
	size_t graph_size;		/* The width of the graph array. */
	if (opt_rev_graph && commit->graph_size) {
		size_t i;

		wmove(view->win, lineno, col);
		/* Using waddch() instead of waddnstr() ensures that
		 * they'll be rendered correctly for the cursor line. */
		for (i = 0; i < commit->graph_size; i++)
			waddch(view->win, commit->graph[i]);

		col += commit->graph_size + 1;
	}

	wmove(view->win, lineno, col);
		commit->graph[commit->graph_size++] = ACS_LTEE;
	{ 'g',		REQ_TOGGLE_REV_GRAPH},