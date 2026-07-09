#include <search.h>
#include <stdlib.h>

// RB Tree implementation based on the code in wikipedia https://en.wikipedia.org/wiki/Red%E2%80%93black_tree

struct __posix_tnode
{
	const void* key;
	posix_tnode* parent;
	posix_tnode* children[2];
	enum { RED, BLACK } color;
};

posix_tnode* tfind(const void* key, posix_tnode* const* rootp, int (*compar)(const void*, const void*))
{
	if (*rootp == nullptr)
		return nullptr;

	posix_tnode* node = *rootp;
	while (node)
	{
		const int result = compar(key, node->key);
		if (result == 0)
			return node;
		node = node->children[result > 0];
	}

	return nullptr;
}

static posix_tnode* _trotate(posix_tnode** root, posix_tnode* node, int dir)
{
	posix_tnode* old_parent = node->parent;
	posix_tnode* new_root   = node->children[1 - dir];
	posix_tnode* new_child  = new_root->children[dir];

	node->children[1 - dir] = new_child;

	if (new_child)
		new_child->parent = node;

	new_root->children[dir] = node;

	new_root->parent = old_parent;
	node->parent = new_root;
	if (old_parent)
		old_parent->children[node == old_parent->children[1]] = new_root;
	else
		*root = new_root;

	return new_root;
}

static void _tinsert(posix_tnode** root, posix_tnode* node, posix_tnode* parent, int dir)
{
	node->color = posix_tnode::RED;
	node->parent = parent;

	if (!parent)
	{
		*root = node;
		return;
	}

	parent->children[dir] = node;

	do {
		if (parent->color == posix_tnode::BLACK)
			return;

		posix_tnode* grandparent = parent->parent;
		if (!grandparent)
		{
			parent->color = posix_tnode::BLACK;
			return;
		}

		dir = (parent == grandparent->children[1]);

		posix_tnode* uncle = grandparent->children[1 - dir];
		if (!uncle || uncle->color == posix_tnode::BLACK)
		{
			if (node == parent->children[1 - dir])
			{
				_trotate(root, parent, dir);
				node = parent;
				parent = grandparent->children[dir];
			}

			_trotate(root, grandparent, 1 - dir);
			parent->color = posix_tnode::BLACK;
			grandparent->color = posix_tnode::RED;
			return;
		}

		parent->color = posix_tnode::BLACK;
		uncle->color = posix_tnode::BLACK;
		grandparent->color = posix_tnode::RED;
		node = grandparent;
	} while ((parent = node->parent));
}

posix_tnode* tsearch(const void* key, posix_tnode** root, int (*compar)(const void*, const void*))
{
	posix_tnode* parent = nullptr;
	posix_tnode* node = *root;
	int dir = 0;

	while (node)
	{
		const int result = compar(key, node->key);
		if (result == 0)
			return node;
		parent = node;
		dir = (result > 0);
		node = node->children[dir];
	}

	node = static_cast<posix_tnode*>(malloc(sizeof(posix_tnode)));
	if (node == nullptr)
		return nullptr;

	*node = {
		.key = key,
		.parent = nullptr,
		.children = {},
		.color = posix_tnode::RED,
	};

	_tinsert(root, node, parent, dir);

	return node;
}

static void _tremove_black_leaf(posix_tnode** __restrict root, posix_tnode* node)
{
	const auto case6 = [&](posix_tnode* parent, posix_tnode* sibling, posix_tnode* far_nephew, int dir) -> bool {
		_trotate(root, parent, dir);
		sibling->color = parent->color;
		parent->color = posix_tnode::BLACK;
		far_nephew->color = posix_tnode::BLACK;
		return true;
	};

	const auto case5 = [&](posix_tnode* parent, posix_tnode* sibling, posix_tnode* near_nephew, int dir) -> bool {
		_trotate(root, sibling, 1 - dir);
		sibling->color = posix_tnode::RED;
		near_nephew->color = posix_tnode::BLACK;
		case6(parent, near_nephew, sibling, dir);
		return true;
	};

	const auto try_case56 = [&](posix_tnode* parent, posix_tnode* sibling, int dir) -> bool {
		if (posix_tnode* far_nephew  = sibling->children[1 - dir]; far_nephew  && far_nephew->color  == posix_tnode::RED)
			return case6(parent, sibling, far_nephew, dir);
		if (posix_tnode* near_nephew = sibling->children[    dir]; near_nephew && near_nephew->color == posix_tnode::RED)
			return case5(parent, sibling, near_nephew, dir);
		return false;
	};

	posix_tnode* parent = node->parent;

	int dir = (node == parent->children[1]);

	parent->children[dir] = nullptr;

	for (;;)
	{
		posix_tnode* sibling = parent->children[1 - dir];

		if (sibling->color == posix_tnode::RED)
		{
			posix_tnode* near_nephew = sibling->children[dir];

			_trotate(root, parent, dir);
			parent->color = posix_tnode::RED;
			sibling->color = posix_tnode::BLACK;
			sibling = near_nephew;

			if (try_case56(parent, sibling, dir))
				return;

			sibling->color = posix_tnode::RED;
			parent->color = posix_tnode::BLACK;
			return;
		}

		if (try_case56(parent, sibling, dir))
			return;

		if (parent->color == posix_tnode::RED)
		{
			sibling->color = posix_tnode::RED;
			parent->color = posix_tnode::BLACK;
			return;
		}

		sibling->color = posix_tnode::RED;

		node = parent;
		parent = node->parent;
		if (parent == nullptr)
			return;

		dir = (node == parent->children[1]);
	}
}

void* tdelete(const void* __restrict key, posix_tnode** __restrict root, int (*compar)(const void*, const void*))
{
	posix_tnode* node = tfind(key, root, compar);
	if (node == nullptr)
		return nullptr;

	// The tdelete() function shall return a pointer to the parent of the deleted node, or an unspecified non-null pointer
	// if the deleted node was the root node, or a null pointer if the node is not found.
	static posix_tnode dummy {};
	posix_tnode* return_value = node->parent ? node->parent : &dummy;

	if (node->children[0] && node->children[1])
	{
		posix_tnode* succ = node->children[1];
		while (succ->children[0])
			succ = succ->children[0];
		node->key = succ->key;
		node = succ;
	}

	posix_tnode* parent = node->parent;
	posix_tnode* child = node->children[0]
		? node->children[0]
		: node->children[1];

	if (parent == nullptr || child)
	{
		posix_tnode* parent = node->parent;

		if (child)
		{
			child->parent = parent;
			child->color = posix_tnode::BLACK;
		}

		if (parent == nullptr)
			*root = child;
		else
		{
			int dir = (node == parent->children[1]);
			parent->children[dir] = child;
		}
	}
	else if (node->color == posix_tnode::RED)
	{
		int dir = (node == parent->children[1]);
		parent->children[dir] = nullptr;
	}
	else
	{
		_tremove_black_leaf(root, node);
	}

	free(node);

	return return_value;
}

static void _twalk(const posix_tnode* node, void (*action)(const posix_tnode*, VISIT, int), int depth)
{
	if (!node->children[0] && !node->children[1])
		return action(node, leaf, depth);
	action(node, preorder, depth);
	if (node->children[0])
		_twalk(node->children[0], action, depth + 1);
	action(node, postorder, depth);
	if (node->children[1])
		_twalk(node->children[1], action, depth + 1);
	action(node, endorder, depth);
}

void twalk(const posix_tnode* root, void (*action)(const posix_tnode*, VISIT, int))
{
	if (root == nullptr)
		return;
	_twalk(root, action, 0);
}
