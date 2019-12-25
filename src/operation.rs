#[derive(PartialEq, Debug, Clone)]
enum Op {
    Retain(usize),
    Insert(String),
    Delete(usize),
}

#[derive(PartialEq, Debug, Clone)]
struct TextOperation {
    // When an operation is applied to an input string, you can think of this as if an imaginary
    // cursor runs over the entire string and skips over some parts, deletes some parts and inserts
    // characters at some positions. These actions (skip/delete/insert) are stored as an array in
    // the "ops" property.
    ops: Vec<Op>,

    // An operation's base_length is the length of every string the operation can be applied to.
    base_length: usize,

    // The target_length is the length of every string that results from applying the operation on
    // a valid input string.
    target_length: usize,
}

impl TextOperation {
    // Create a new, empty TextOperation
    pub fn new() -> Self {
        Self {
            ops: vec![],
            base_length: 0,
            target_length: 0,
        }
    }

    // Add a retain op to this TextOperation (used in the builder pattern)
    pub fn retain(mut self: Self, n: usize) -> Self {
        if n == 0 {
            return self;
        }

        self.base_length += n;
        self.target_length += n;

        if let Some(&mut Op::Retain(ref mut l)) = self.ops.last_mut() {
            *l += n;
        } else {
            self.ops.push(Op::Retain(n));
        }

        self
    }

    // Add an insert op to this TextOperation (used in the builder pattern)
    pub fn insert(mut self: Self, s: impl Into<String>) -> Self {
        let s = s.into();
        let l = s.len();

        if  l == 0 {
            return self;
        }

        if let Some(&Op::Delete(_)) = self.ops.last() {
            // maintain the invariant that inserts never follow a delete by popping the
            // delete. adding the insert, and then re-adding the delete
            let del = self.ops.pop().unwrap();
            self = self.insert(s);
            self.ops.push(del);
            return self;
        }

        self.target_length += l;

        if let Some(&mut Op::Insert(ref mut l)) = self.ops.last_mut() {
            l.push_str(&s)
        } else {
            self.ops.push(Op::Insert(s));
        }

        self
    }

    // Add a delete op to this TextOperation (used in the builder pattern)
    pub fn delete(mut self: Self, n: usize) -> Self {
        if n == 0 {
            return self;
        }

        self.base_length += n;

        if let Some(&mut Op::Delete(ref mut l)) = self.ops.last_mut() {
            *l += n;
        } else {
            self.ops.push(Op::Delete(n));
        }

        self
    }

    // Apply an operation to a string. Returns an error if there's a mismatch between the input string
    // and the operation.
    pub fn apply(self: &Self, input: &str) -> Result<String, String> {
        let strlen = input.len();

        if strlen != self.base_length {
            return Err(String::from("The operation's base length must be equal to the string's length"))
        }

        let mut res = String::new();
        let mut i = 0;
        for op in &self.ops {
            match op {
                Op::Retain(n) => {
                    assert!(i + n <= strlen);
                    res.push_str(&input[i..i + n]);
                    i += n;
                },
                Op::Insert(s) => {
                    res.push_str(&s);
                },
                Op::Delete(n) => {
                    i += n;
                },
            }
        }

        Ok(res)
    }

    // Transform takes two operations A and B that happened concurrently and produces two
    // operations A' and B' such that `apply(apply(S, A), B') = apply(apply(S, B), A')`. This
    // function is the heart of OT.  That is, given a state map
    //
    //        *
    //       / \
    //  op1 /   \ op2
    //     /     \
    //    *       *
    //
    // this function "completes the diamond:
    //
    //    *       *
    //     \     /
    // op2' \   / op1'
    //       \ /
    //        *
    // 
    // such that applying op2' after op1 has the same effect as applying op1' after op2.  This
    // allows two different systems which have already applied op1 and op2, respectively, and thus
    // reached different states, to return to the same state by applying op2' and op1',
    // respectively.
    pub fn transform(mut operation1: TextOperation, mut operation2: TextOperation) -> (TextOperation, TextOperation) {
        // both must start at the same state, and thus have the same base length
        assert_eq!(operation1.base_length, operation2.base_length);

        let (mut operation1p, mut operation2p) = (TextOperation::new(), TextOperation::new());
        let (mut ops1, mut ops2) = (operation1.ops.iter_mut(), operation2.ops.iter_mut());
        let (mut op1, mut op2) = (ops1.next(), ops2.next());

        loop {
            match (&mut op1, &mut op2) {
                // end condition: both ops1 and ops2 have been processed
                (None, None) => break,

                // handle inserts, preferring op1 if both are inserts
                (Some(&mut Op::Insert(ref s)), _) => {
                    operation1p = operation1p.insert(s.clone());
                    operation2p = operation2p.retain(s.len());
                    op1 = ops1.next();
                    continue;
                },

                (_, Some(&mut Op::Insert(ref s))) => {
                    operation1p = operation1p.retain(s.len());
                    operation2p = operation2p.insert(s.clone());
                    op2 = ops2.next();
                    continue;
                },

                (None, _) | (_, None) => {
                    unreachable!();
                }

                (Some(&mut Op::Retain(ref mut n1)), Some(&mut Op::Retain(ref mut n2))) => {
                    // retain the minimum of the two, and rewrite the larger input op
                    // to only retain what's left
                    let min;
                    if n1 > n2 {
                        min = *n2;
                        *n1 = *n1 - *n2;
                        op2 = ops2.next();
                    } else if n1 == n2 {
                        min = *n2;
                        op1 = ops1.next();
                        op2 = ops2.next();
                    } else {
                        min = *n1;
                        *n2 = *n2 - *n1;
                        op1 = ops1.next();
                    }
                    operation1p = operation1p.retain(min);
                    operation2p = operation2p.retain(min);
                },
                (Some(Op::Delete(n1)), Some(Op::Delete(n2))) => {
                    // Both operations delete the same string at the same position. We don't need
                    // to produce any operations, we just skip over the delete ops and handle the
                    // case that one operation deletes more than the other.
                    if n1 > n2 {
                        *n1 = *n2 - *n1;
                        op2 = ops2.next();
                    } else if n1 == n2 {
                        op1 = ops1.next();
                        op2 = ops2.next();
                    } else {
                        *n2 = *n2 - *n1;
                        op1 = ops1.next();
                    }
                },
                (Some(Op::Delete(n1)), Some(Op::Retain(n2))) => {
                    let min;
                    if n1 > n2 {
                        min = *n2;
                        *n1 = *n1 - *n2;
                        op2 = ops2.next();
                    } else if n1 == n2 {
                        min = *n1;
                        op1 = ops1.next();
                        op2 = ops2.next();
                    } else {
                        min = *n1;
                        *n2 = *n2 - *n1;
                        op1 = ops1.next();
                    }
                    operation1p = operation1p.delete(min);
                },
                (Some(Op::Retain(n1)), Some(Op::Delete(n2))) => {
                    let min;
                    if n1 > n2 {
                        min = *n2;
                        *n1 = *n1 - *n2;
                        op2 = ops2.next();
                    } else if n1 == n2 {
                        min = *n1;
                        op1 = ops1.next();
                        op2 = ops2.next();
                    } else {
                        min = *n1;
                        *n2 = *n2 - *n1;
                        op1 = ops1.next();
                    }
                    operation2p = operation2p.delete(min);
                },
            }
        }

        (operation1p, operation2p)
    }
}

#[cfg(test)]
mod test {
    use super::{TextOperation, Op};

    #[test]
    fn test_build_retain() {
        let op = TextOperation::new()
            .retain(10);
        assert_eq!(op.base_length, 10);
        assert_eq!(op.target_length, 10);
        assert_eq!(op.ops, vec![Op::Retain(10)]);
    }

    #[test]
    fn test_build_retain_null() {
        let op = TextOperation::new()
            .retain(0);
        assert_eq!(op.base_length, 0);
        assert_eq!(op.target_length, 0);
        assert_eq!(op.ops, vec![]);
    }

    #[test]
    fn test_build_retain_merge() {
        let op = TextOperation::new()
            .retain(10)
            .retain(20);
        assert_eq!(op.base_length, 30);
        assert_eq!(op.target_length, 30);
        assert_eq!(op.ops, vec![Op::Retain(30)]);
    }

    #[test]
    fn test_build_retain_after_insert() {
        let op = TextOperation::new()
            .insert(String::from("hello"))
            .retain(20);
        assert_eq!(op.base_length, 20);
        assert_eq!(op.target_length, 25);
        assert_eq!(op.ops, vec![Op::Insert(String::from("hello")), Op::Retain(20)]);
    }

    #[test]
    fn test_build_retain_after_delete() {
        let op = TextOperation::new()
            .delete(10)
            .retain(20);
        assert_eq!(op.base_length, 30);
        assert_eq!(op.target_length, 20);
        assert_eq!(op.ops, vec![Op::Delete(10), Op::Retain(20)]);
    }

    #[test]
    fn test_build_insert() {
        let op = TextOperation::new()
            .insert(String::from("hello"));
        assert_eq!(op.base_length, 0);
        assert_eq!(op.target_length, 5);
        assert_eq!(op.ops, vec![Op::Insert(String::from("hello"))]);
    }

    #[test]
    fn test_build_insert_null() {
        let op = TextOperation::new()
            .insert(String::from(""));
        assert_eq!(op.base_length, 0);
        assert_eq!(op.target_length,0);
        assert_eq!(op.ops, vec![]);
    }

    #[test]
    fn test_build_insert_merge() {
        let op = TextOperation::new()
            .insert(String::from("hello"))
            .insert(String::from(" world"));
        assert_eq!(op.base_length, 0);
        assert_eq!(op.target_length, 11);
        assert_eq!(op.ops, vec![Op::Insert(String::from("hello world"))]);
    }

    #[test]
    fn test_build_insert_after_retain() {
        let op = TextOperation::new()
            .retain(10)
            .insert(String::from("hello"));
        assert_eq!(op.base_length, 10);
        assert_eq!(op.target_length, 15);
        assert_eq!(op.ops, vec![Op::Retain(10), Op::Insert(String::from("hello"))]);
    }

    #[test]
    fn test_build_insert_after_delete() {
        let op = TextOperation::new()
            .delete(10)
            .insert(String::from("hello"));
        assert_eq!(op.base_length, 10);
        assert_eq!(op.target_length, 5);
        assert_eq!(op.ops, vec![
            Op::Insert(String::from("hello")),
            Op::Delete(10),
        ]);
    }

    #[test]
    fn test_build_insert_after_delete_merge() {
        let op = TextOperation::new()
            .insert(String::from("hello"))
            .delete(10)
            .insert(String::from(" world"));
        assert_eq!(op.base_length, 10);
        assert_eq!(op.target_length, 11);
        assert_eq!(op.ops, vec![
            Op::Insert(String::from("hello world")),
            Op::Delete(10),
        ]);
    }

    #[test]
    fn test_build_delete() {
        let op = TextOperation::new()
            .delete(17);
        assert_eq!(op.base_length, 17);
        assert_eq!(op.target_length, 0);
        assert_eq!(op.ops, vec![Op::Delete(17)]);
    }

    #[test]
    fn test_build_delete_null() {
        let op = TextOperation::new()
            .delete(0);
        assert_eq!(op.base_length, 0);
        assert_eq!(op.target_length, 0);
        assert_eq!(op.ops, vec![]);
    }

    #[test]
    fn test_build_delete_merge() {
        let op = TextOperation::new()
            .delete(5)
            .delete(8);
        assert_eq!(op.base_length, 13);
        assert_eq!(op.target_length, 0);
        assert_eq!(op.ops, vec![Op::Delete(13)]);
    }

    #[test]
    fn test_build_delete_after_insert() {
        let op = TextOperation::new()
            .insert(String::from("hello"))
            .delete(8);
        assert_eq!(op.base_length, 8);
        assert_eq!(op.target_length, 5);
        assert_eq!(op.ops, vec![Op::Insert(String::from("hello")), Op::Delete(8)]);
    }

    #[test]
    fn test_build_delete_after_retain() {
        let op = TextOperation::new()
            .retain(10)
            .delete(8);
        assert_eq!(op.base_length, 18);
        assert_eq!(op.target_length, 10);
        assert_eq!(op.ops, vec![Op::Retain(10), Op::Delete(8)]);
    }

    #[test]
    fn test_apply_retain() {
        let op = TextOperation::new()
            .retain(5);
        assert_eq!(
            op.apply("hello"),
            Ok(String::from("hello")));
    }

    #[test]
    fn test_apply_insert() {
        let op = TextOperation::new()
            .retain(5)
            .insert(String::from(" cruel"))
            .retain(6);
        assert_eq!(
            op.apply("hello world"),
            Ok(String::from("hello cruel world")));
    }

    #[test]
    fn test_apply_delete() {
        let op = TextOperation::new()
            .retain(5)
            .delete(6)
            .retain(6);
        assert_eq!(
            op.apply("hello cruel world"),
            Ok(String::from("hello world")));
    }

    #[test]
    fn test_apply_wrong_length() {
        let op = TextOperation::new()
            .retain(5)
            .insert(String::from(" cruel"))
            .retain(6);
        assert_eq!(
            op.apply("hello cruel world"),
            Err(String::from("The operation's base length must be equal to the string's length")));
    }

    mod transform {
        use super::{TextOperation};

        const STARTING_STATE: &str ="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

        fn test_transform(o1: TextOperation, o2: TextOperation, exp1p: TextOperation, exp2p: TextOperation) {
            // first check that the resulting operation is as expected..
            let (o1p, o2p) = TextOperation::transform(o1.clone(), o2.clone());
            assert_eq!((&o1p, &o2p), (&exp1p, &exp2p));

            // then check that the definition of `transform` is satisfied, by applying to an
            // arbitrary string
            let input = &STARTING_STATE[0..o1.base_length];
            // B' composed with A
            let first = o2p.apply(&o1.apply(&input).unwrap()).unwrap();
            // A' composed with B
            let second = o1p.apply(&o2.apply(&input).unwrap()).unwrap();
            print!("{} -> {}\n", input, first);
            assert_eq!(first, second);
        }

        #[test]
        fn test_transform_empty() {
            test_transform(
                TextOperation::new(),
                TextOperation::new(),
                TextOperation::new(),
                TextOperation::new());
        }

        #[test]
        fn test_transform_one_noop() {
            test_transform(
                TextOperation::new().retain(2).insert("123").retain(10),
                TextOperation::new().retain(12),
                TextOperation::new().retain(2).insert("123").retain(10),
                TextOperation::new().retain(15));
        }

        #[test]
        fn test_transform_two_inserts() {
            test_transform(
                TextOperation::new().insert("123"),
                TextOperation::new().insert("567"),
                TextOperation::new().insert("123").retain(3),
                TextOperation::new().retain(3).insert("567"));
        }

        #[test]
        fn test_transform_two_retains() {
            test_transform(
                TextOperation::new().retain(10),
                TextOperation::new().retain(10),
                TextOperation::new().retain(10),
                TextOperation::new().retain(10));
        }

        #[test]
        fn test_transform_insert_two_different_spots() {
            test_transform(
                TextOperation::new().retain(5).insert("123").retain(10),
                TextOperation::new().retain(10).insert("567").retain(5),
                TextOperation::new().retain(5).insert("123").retain(13),
                TextOperation::new().retain(13).insert("567").retain(5));
        }

        #[test]
        fn test_transform_insert_two_different_spots_reversed() {
            test_transform(
                TextOperation::new().retain(10).insert("567").retain(5),
                TextOperation::new().retain(5).insert("123").retain(10),
                TextOperation::new().retain(13).insert("567").retain(5),
                TextOperation::new().retain(5).insert("123").retain(13));
        }

        #[test]
        fn test_transform_two_deletes() {
            test_transform(
                TextOperation::new().delete(10),
                TextOperation::new().delete(10),
                TextOperation::new(),
                TextOperation::new());
        }

        #[test]
        fn test_transform_delete_retain() {
            test_transform(
                TextOperation::new().retain(10),
                TextOperation::new().delete(3).retain(7),
                TextOperation::new().retain(7),
                TextOperation::new().delete(3).retain(7));
        }

        #[test]
        fn test_transform_retain_delete() {
            test_transform(
                TextOperation::new().delete(3).retain(7),
                TextOperation::new().retain(10),
                TextOperation::new().delete(3).retain(7),
                TextOperation::new().retain(7));
        }

        #[test]
        fn test_transform_delete_insert() {
            test_transform(
                TextOperation::new().delete(10),
                TextOperation::new().retain(3).insert("123").retain(7),
                TextOperation::new().delete(3).retain(3).delete(7),
                TextOperation::new().insert("123"));
        }
    }
}
