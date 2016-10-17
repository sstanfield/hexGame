tb_addWidget("RootWindow", [[TBLayout: id: main_layout, axis: y, distribution: available, spacing: 5]]);
tb_addWidget("main_layout", [[TBLayout: id: main_layout1, axis: x, distribution: available, spacing: 5]]);
tb_addWidget("main_layout", [[TBLayout: id: main_layout2, axis: x, distribution: available, spacing: 5]]);

h = tb_getHeight("main_layout");
w = tb_getWidth("main_layout");
mw = math.floor(w*.8);
mh = math.floor(h*.7);

tb_addWidget("main_layout1", [[
MapWidget: id: main_map distribution: available, spacing: 5, allowSelect: 1, is-focusable: 1
	lp: width: ]]..(mw)..[[px, height: ]]..(mh)..[[px
]]);
tb_addWidget("main_layout1", [[TBLayout: id: main_sidebar, axis: y, distribution: available, spacing: 5]]);

tb_addWidget("main_sidebar", [[
MiniMapWidget: id: mini_map distribution: available, spacing: 5, allowSelect: 1, is-focusable: 1
	lp: width: ]]..(w-mw)..[[px, height: ]]..(h-mh)..[[px
]]);
