import casadi as ca


def main():
    # Symbolic expression
    x = ca.MX.sym("x")
    f = x**2 + 3 * x + 2

    # Compute the gradient
    dfdx = ca.gradient(f, x)
    dfdx_func = ca.Function("dfdx_func", [x], [dfdx])

    # Evaluate the gradient at a specific point
    evaluation_point = 2
    print("Function: ", f)
    print("Gradient: ", dfdx)
    print(f"Derivative at x={evaluation_point}: ", dfdx_func(evaluation_point))


if __name__ == "__main__":
    main()
