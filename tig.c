#define TIG_LS_REMOTE \
	"git ls-remote . 2>/dev/null"

#define TIG_DIFF_CMD \
	"git show --patch-with-stat --find-copies-harder -B -C %s"

#define TIG_LOG_CMD	\
	"git log --cc --stat -n100 %s"

#define TIG_MAIN_CMD \
	"git log --topo-order --stat --pretty=raw %s"

/* XXX: Needs to be defined to the empty string. */
#define TIG_HELP_CMD	""
#define TIG_PAGER_CMD	""

static bool
string_nformat(char *buf, size_t bufsize, int *bufpos, const char *fmt, ...)
{
	va_list args;
	int pos = bufpos ? *bufpos : 0;

	va_start(args, fmt);
	pos += vsnprintf(buf + pos, bufsize - pos, fmt, args);
	va_end(args);

	if (bufpos)
		*bufpos = pos;

	return pos >= bufsize ? FALSE : TRUE;
}

#define string_format(buf, fmt, args...) \
	string_nformat(buf, sizeof(buf), NULL, fmt, args)

#define string_format_from(buf, from, fmt, args...) \
	string_nformat(buf, sizeof(buf), from, fmt, args)
/*
 * Options
 */
enum option_type {
	OPT_NONE,
	OPT_INT,
};

static bool
check_option(char *opt, char short_name, char *name, enum option_type type, ...)
{
	va_list args;
	char *value = "";
	int *number;

	if (opt[0] != '-')
		return FALSE;

	if (opt[1] == '-') {
		int namelen = strlen(name);

		opt += 2;

		if (strncmp(opt, name, namelen))
			return FALSE;

		if (opt[namelen] == '=')
			value = opt + namelen + 1;

	} else {
		if (!short_name || opt[1] != short_name)
			return FALSE;
		value = opt + 2;
	}

	va_start(args, type);
	if (type == OPT_INT) {
		number = va_arg(args, int *);
		if (isdigit(*value))
			*number = atoi(value);
	}
	va_end(args);

	return TRUE;
}

		if (check_option(opt, 'n', "line-number", OPT_INT, &opt_num_interval)) {
		if (check_option(opt, 'b', "tab-size", OPT_INT, &opt_tab_size)) {
			opt_tab_size = MIN(opt_tab_size, TABSIZE);
		if (check_option(opt, 'v', "version", OPT_NONE)) {
		if (check_option(opt, 'h', "help", OPT_NONE)) {
LINE(PP_REFS,	   "Refs: ",		COLOR_RED,	COLOR_DEFAULT,	0), \
static struct int_map color_map[] = {
#define COLOR_MAP(name) { #name, STRING_SIZE(#name), COLOR_##name }
	COLOR_MAP(DEFAULT),
	COLOR_MAP(BLACK),
	COLOR_MAP(BLUE),
	COLOR_MAP(CYAN),
	COLOR_MAP(GREEN),
	COLOR_MAP(MAGENTA),
	COLOR_MAP(RED),
	COLOR_MAP(WHITE),
	COLOR_MAP(YELLOW),
};

static struct int_map attr_map[] = {
#define ATTR_MAP(name) { #name, STRING_SIZE(#name), A_##name }
	ATTR_MAP(NORMAL),
	ATTR_MAP(BLINK),
	ATTR_MAP(BOLD),
	ATTR_MAP(DIM),
	ATTR_MAP(REVERSE),
	ATTR_MAP(STANDOUT),
	ATTR_MAP(UNDERLINE),
};

	if (!home || !string_format(buf, "%s/.tigrc", home))
	bool (*read)(struct view *view, char *data);
		if (!string_format(view->cmd, format, id, id, id, id, id))
		if (!view->ops->read(view, line))
	if (view == VIEW(REQ_VIEW_HELP)) {
		load_help_page();

	} else if ((reload || strcmp(view->vid, view->id)) &&
		   !begin_update(view)) {
static void
add_pager_refs(struct view *view, struct line *line)
{
	char buf[1024];
	char *data = line->data;
	struct ref **refs;
	int bufpos = 0, refpos = 0;
	const char *sep = "Refs: ";

	assert(line->type == LINE_COMMIT);

	refs = get_refs(data + STRING_SIZE("commit "));
	if (!refs)
		return;

	do {
		struct ref *ref = refs[refpos];
		char *fmt = ref->tag ? "%s[%s]" : "%s%s";

		if (!string_format_from(buf, &bufpos, fmt, sep, ref->name))
			return;
		sep = ", ";
	} while (refs[refpos++]->next);

	if (!realloc_lines(view, view->line_size + 1))
		return;

	line = &view->line[view->lines];
	line->data = strdup(buf);
	if (!line->data)
		return;

	line->type = LINE_PP_REFS;
	view->lines++;
}

pager_read(struct view *view, char *data)
	struct line *line = &view->line[view->lines];
	line->data = strdup(data);
	if (!line->data)
		return FALSE;
	line->type = get_line_type(line->data);

	if (line->type == LINE_COMMIT &&
	    (view == VIEW(REQ_VIEW_DIFF) ||
	     view == VIEW(REQ_VIEW_LOG)))
		add_pager_refs(view, line);

main_read(struct view *view, char *line)
	struct commit *commit = view->lines
			      ? view->line[view->lines - 1].data : NULL;
		if (!commit)
		if (!commit)
		if (!string_format_from(buf, &pos, "%s%s", sep, seq))
	pager_read(view, "Quick reference for tig keybindings:");
			pager_read(view, "");
			pager_read(view, req_info[i].help);
		if (!string_format(buf, "%-25s %s", key, req_info[i].help))
		pager_read(view, buf);
	if (!strncmp(name, "refs/tags/", STRING_SIZE("refs/tags/"))) {
		/* Commits referenced by tags has "^{}" appended. */
		if (name[namelen - 1] != '}')
			return OK;

		namelen -= STRING_SIZE("refs/tags/");
		name	+= STRING_SIZE("refs/tags/");
		namelen -= STRING_SIZE("refs/heads/");
		name	+= STRING_SIZE("refs/heads/");
	ref->name = malloc(namelen + 1);
	strncpy(ref->name, name, namelen);
	ref->name[namelen] = 0;
	if (!strcmp(name, "i18n.commitencoding"))