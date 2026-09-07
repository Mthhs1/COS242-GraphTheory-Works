
class Node():
    def __init__(self, item):
        self.item = item
        self.next = None

class Stack():
    def __init__(self):
        
        self.head: Node | None = None
        
    def push(self, item):
        
        node = Node(item)
        
        if self.head == None:
            self.head = node
            return
        
        bottom = self.head
        self.head = node
        self.head.next = bottom
        return
        
    def pop(self):
        popped = self.head
        self.head = self.head.next
        return popped.item
    
    def isEmpty(self):
        if self.head == None:
            return True
        return False 
        
    def length(self):
        n = 0
        
        if self.isEmpty():
            return n
        
        actual = self.head
        
        while actual != None:
            n += 1
            
            actual = actual.next
            
        return n
    
    def top(self):
        return self.head.item
    
    
if __name__ == "__main__":
    stack1 = Stack()
    stack1.push(1)
    stack1.push(2)
    print(stack1.pop())
    print(stack1.isEmpty())
    print(stack1.length())
    print(stack1.top())
