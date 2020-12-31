local bindings = {
	{ 'n', "<F1>", ":make!<cr>", { noremap = false, silent = false}},
	{ 'n', "<F2>", ":!premake5 gmake2<cr>", { noremap = false, silent = false}},
	{ 'n', "<F3>", ":!blasm examples/fibonacci.blasm -o /tmp/fib.bl", { noremap = false, silent = false}},
	{ 'n', "<F4>", ":!blvi /tmp/fib.bl", { noremap = false, silent = false}},
	{ 'n', "<F5>", ":!bear -- make<cr>", { noremap = false, silent = true}},
}

for _, bind in ipairs(bindings) do
	vim.api.nvim_set_keymap(bind[1], bind[2], bind[3], bind[4])
end

vim.o.errorformat = table.concat(
	{
		"../%f:%l:%c: %trreur: %m",
		"../%f:%l:%c: %tttention: %m",
		"../%f:%l:%c: %m"
	},
	','
)
