print("in main.lua");
tb_addWidget("RootWindow", [[TBLayout: id: main_layout1, axis: y, distribution: available, spacing: 5]]);

h = tb_getHeight("main_layout1");
w = tb_getWidth("main_layout1");
mw = math.floor(w*.8);
mh = math.floor(h*.7);

tb_addWidget("main_layout1", [[
MapWidget: id: main_map distribution: available, spacing: 5, allowSelect: 1, is-focusable: 1
	lp: width: ]]..(mw)..[[px, height: ]]..(mh)..[[px
]]);

