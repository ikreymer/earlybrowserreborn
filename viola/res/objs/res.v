\name {res}
\class {generic}
\script	{

/*
xx = "\n\n\\p(ht.lines.small.gif)";
print("before {", xx, "}\n");
print("after {", trimEdgeQ(xx), "}\n");
exit(0);

url = "http://pebble/vdoc";
print(">>>>>>>>>>>>>>>>>>>>>>>>>URL=", url, "\n");
x = HTTPGet(url);
normURL = send("normalizeURL", "normalizeURL", url);
print(">>>>>>>>>>>>>>>>>>>>>>>>>URL=", url, "\n");
print(">>>>>>>>>>>>>>>>>>>>>>>>>normURL=", normURL, "\n");

url = "http://pebble";
print(">>>>>>>>>>>>>>>>>>>>>>>>>URL=", url, "\n");
x = HTTPGet(url);
normURL = send("normalizeURL", "normalizeURL", url);
print(">>>>>>>>>>>>>>>>>>>>>>>>>URL=", url, "\n");
print(">>>>>>>>>>>>>>>>>>>>>>>>>normURL=", normURL, "\n");

exit(0);
*/
/*
print("start-------\n");
for (i = 0; i < 1000; i++) {
	lappend(i, 0);
	xxx n[i] = 0;
}

print("end---------\n");
exit(0);
*/
	switch (arg[0]) {
	case "targetSet":
		shell("targetSet");
		return;
	break;
	case "incAppCount":
		++appCount;
		return;
	break;
	case "decAppCount":
	case "quit":
		--appCount;
		if (appCount <= 0) quit();
		return;
	break;
/*
	case "fifi":
		x += 10;
		return x;
	break;
	case "init":
		usual();
		x = 1;
		send(self(), "foo", x, x += 100,
			send(self(), "fifi"));
	break;
	case "foo":
		print("FOO:\n");
		print("arg[1]=", arg[1], "\n");
		print("arg[2]=", arg[2], "\n");
		print("arg[3]=", arg[3], "\n");
		return;
	break;
*/
	}
	usual();
}
\
