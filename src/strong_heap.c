// Strong binary heap properties
// Root node is largest, right node is always larger than left


//Pseduo-code

construct in an array (inserts from bottom of heap)

balance_down(index)
{
	if (root > right child) || (root is leaf)
		return
	swap root with right child
	if left child > right child {
		swap left child with right child
		balance_down(left_child_index)
	}
	balance_down(right_child_index)
}


construct(node_index)
{
	// Remember, node is a leaf when we start
	if node <= parent {
		if (node is left-child) {
			done
		}
		if (node < left child) {
			swap node with left-child
			balance-down starting from node
		}
		done
	}
	// Ripple up
	while (node > parent) {
		swap(node, parent)
		if (parent is now a left node) { // if parent index & 1
			if (parent > right node)
				swap parent with right node
		}
	}
}
	
