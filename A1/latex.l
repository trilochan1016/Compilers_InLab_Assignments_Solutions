%{
#define CMD 1
#define ENV 2
#define MTH 3
#define DSP 4
#define OTH 5

int mth = 0, dsp = 0, env = 0;
%}

space	[ \t\n]
spaces	{space}+
letter	[a-zA-Z]
active	[~]
bs	[\\]
other	[^a-zA-Z~]
nocmd	[^\\~\$%]+
benv	\\begin{space}*\{{letter}+\}
eenv	\\end{space}*\{{letter}+\}
cmd	{bs}({other}|({letter})+)
bcmt	"%"
cmt	{bcmt}.*
dlr	"$"
ddlr	"$$"

%%

{cmt}		{ }
{nocmd}		{ }
{dlr}		{ mth = 1 - mth; return(MTH); }
{ddlr}		{ dsp = 1 - dsp; return(DSP); }
"\\\["		{ dsp = 1; return(DSP); }
"\\\]"		{ dsp = 0; return(DSP); }
{benv}		{ env = 1; return(ENV); }
{eenv}		{ env = 0; return(ENV); }
{cmd}		{ return(CMD); }
{active}	{ return(CMD); }

%%
