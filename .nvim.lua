local bindings = {
	{ 'n', "<F1>", ":make!<cr>", { noremap = false, silent = false}},
	{ 'n', "<F2>", ":make -C examples -B<cr>", { noremap = false, silent = false}},
	{ 'n', "<F3>", ":!./configure<cr>", { noremap = false, silent = false}},
	{ 'n', "<F4>", ":!./test-x86.sh %<cr>", { noremap = false, silent = false}},
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
