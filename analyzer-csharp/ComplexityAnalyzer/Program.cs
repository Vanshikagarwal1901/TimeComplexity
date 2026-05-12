using System.Text.Json;
using System.Text.RegularExpressions;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;
using Microsoft.CodeAnalysis.CSharp.Syntax;

var code = Console.In.ReadToEnd();
if (string.IsNullOrWhiteSpace(code))
{
    Console.WriteLine(JsonSerializer.Serialize(AnalysisResult.Error("No code was provided.")));
    return;
}

var syntaxTree = CSharpSyntaxTree.ParseText(code);
var root = syntaxTree.GetCompilationUnitRoot();
var diagnostics = syntaxTree.GetDiagnostics()
    .Where(diagnostic => diagnostic.Severity == DiagnosticSeverity.Error)
    .Select(diagnostic => diagnostic.ToString())
    .ToArray();

if (diagnostics.Length > 0)
{
    Console.WriteLine(JsonSerializer.Serialize(AnalysisResult.Error("The submitted code is not valid C#.", diagnostics)));
    return;
}

var analyzer = new ComplexitySyntaxAnalyzer(root);
var result = analyzer.Analyze();
Console.WriteLine(JsonSerializer.Serialize(result, new JsonSerializerOptions { WriteIndented = false }));

public sealed record AnalysisResult(
    string TimeComplexity,
    IReadOnlyList<ComplexityNode> Nodes,
    IReadOnlyList<string> Explanation,
    IReadOnlyList<string> Errors)
{
    public static AnalysisResult Error(string message, IReadOnlyList<string>? details = null)
    {
        var errors = details is null ? [message] : new[] { message }.Concat(details).ToArray();
        return new AnalysisResult("Unable to analyze", [], [], errors);
    }
}

public sealed record ComplexityNode(string Label, string Complexity, int Level, int Depth);

public readonly record struct ComplexityTerm(int Power, int LogPower, bool Unknown = false)
{
    public static ComplexityTerm Constant => new(0, 0);
    public static ComplexityTerm Linear => new(1, 0);
    public static ComplexityTerm Logarithmic => new(0, 1);
    public static ComplexityTerm UnknownTerm => new(1, 0, true);

    public ComplexityTerm Multiply(ComplexityTerm other)
    {
        if (Unknown || other.Unknown)
        {
            return UnknownTerm;
        }

        return new ComplexityTerm(Power + other.Power, LogPower + other.LogPower);
    }

    public static ComplexityTerm Dominant(ComplexityTerm left, ComplexityTerm right)
    {
        if (left.Unknown || right.Unknown)
        {
            return left.Unknown ? left : right;
        }

        var powerComparison = left.Power.CompareTo(right.Power);
        if (powerComparison != 0)
        {
            return powerComparison > 0 ? left : right;
        }

        return left.LogPower >= right.LogPower ? left : right;
    }

    public int Level => Unknown ? 6 : Math.Clamp((Power * 2) + LogPower, 0, 6);

    public override string ToString()
    {
        if (Unknown)
        {
            return "O(n) or worse";
        }

        if (Power == 0 && LogPower == 0)
        {
            return "O(1)";
        }

        var parts = new List<string>();
        if (Power == 1)
        {
            parts.Add("n");
        }
        else if (Power > 1)
        {
            parts.Add($"n^{Power}");
        }

        if (LogPower == 1)
        {
            parts.Add("log n");
        }
        else if (LogPower > 1)
        {
            parts.Add($"log^{LogPower} n");
        }

        return $"O({string.Join(" ", parts)})";
    }
}

public sealed class ComplexitySyntaxAnalyzer
{
    private readonly CompilationUnitSyntax _root;
    private readonly Dictionary<string, MethodDeclarationSyntax> _methods;
    private readonly List<ComplexityNode> _nodes = [];
    private readonly List<string> _explanation = [];

    public ComplexitySyntaxAnalyzer(CompilationUnitSyntax root)
    {
        _root = root;
        _methods = root.DescendantNodes()
            .OfType<MethodDeclarationSyntax>()
            .GroupBy(method => method.Identifier.ValueText)
            .ToDictionary(group => group.Key, group => group.First());
    }

    public AnalysisResult Analyze()
    {
        var entry = _methods.TryGetValue("Main", out var main)
            ? main
            : _methods.Values.FirstOrDefault();

        var complexity = entry is null
            ? AnalyzeStatements(_root.Members.OfType<GlobalStatementSyntax>().Select(statement => statement.Statement), [], 0)
            : AnalyzeMethod(entry, [], 0);

        if (_nodes.Count == 0)
        {
            _nodes.Add(new ComplexityNode("statements", ComplexityTerm.Constant.ToString(), ComplexityTerm.Constant.Level, 0));
            _explanation.Add("No loops or known method calls were found, so the code is treated as constant time.");
        }

        _explanation.Insert(0, $"Dominant term after applying sum/product rules: {complexity}.");
        return new AnalysisResult(complexity.ToString(), _nodes, _explanation.Distinct().ToArray(), []);
    }

    private ComplexityTerm AnalyzeMethod(MethodDeclarationSyntax method, HashSet<string> visiting, int depth)
    {
        var name = method.Identifier.ValueText;
        if (!visiting.Add(name))
        {
            var recursion = EstimateRecursiveMethod(method);
            _nodes.Add(new ComplexityNode($"{name} recursion", recursion.ToString(), recursion.Level, depth));
            _explanation.Add($"Recursive call in {name} is estimated from the argument reduction pattern.");
            return recursion;
        }

        IEnumerable<StatementSyntax> statements = method.Body?.Statements ?? Enumerable.Empty<StatementSyntax>();
        if (!statements.Any() && method.ExpressionBody is not null)
        {
            statements = [SyntaxFactory.ReturnStatement(method.ExpressionBody.Expression)];
        }

        var complexity = AnalyzeStatements(statements, visiting, depth);
        visiting.Remove(name);
        return complexity;
    }

    private ComplexityTerm AnalyzeStatements(IEnumerable<StatementSyntax> statements, HashSet<string> visiting, int depth)
    {
        var total = ComplexityTerm.Constant;
        foreach (var statement in statements)
        {
            var statementComplexity = AnalyzeStatement(statement, visiting, depth);
            total = ComplexityTerm.Dominant(total, statementComplexity);
        }

        return total;
    }

    private ComplexityTerm AnalyzeStatement(StatementSyntax statement, HashSet<string> visiting, int depth)
    {
        return statement switch
        {
            ForStatementSyntax forStatement => AnalyzeLoop("for", EstimateForLoop(forStatement), forStatement.Statement, visiting, depth),
            ForEachStatementSyntax forEachStatement => AnalyzeLoop("foreach", ComplexityTerm.Linear, forEachStatement.Statement, visiting, depth),
            WhileStatementSyntax whileStatement => AnalyzeLoop("while", EstimateWhileLoop(whileStatement), whileStatement.Statement, visiting, depth),
            DoStatementSyntax doStatement => AnalyzeLoop("do/while", EstimateDoLoop(doStatement), doStatement.Statement, visiting, depth),
            IfStatementSyntax ifStatement => ComplexityTerm.Dominant(
                AnalyzeStatement(ifStatement.Statement, visiting, depth),
                ifStatement.Else is null ? ComplexityTerm.Constant : AnalyzeStatement(ifStatement.Else.Statement, visiting, depth)),
            BlockSyntax block => AnalyzeStatements(block.Statements, visiting, depth),
            LocalFunctionStatementSyntax localFunction => AnalyzeLocalFunction(localFunction, visiting, depth),
            _ => AnalyzeInvocations(statement, visiting, depth)
        };
    }

    private ComplexityTerm AnalyzeLoop(string label, ComplexityTerm loopCost, StatementSyntax body, HashSet<string> visiting, int depth)
    {
        var bodyCost = AnalyzeStatement(body, visiting, depth + 1);
        var total = loopCost.Multiply(bodyCost);

        _nodes.Add(new ComplexityNode(label, total.ToString(), total.Level, depth));
        if (loopCost.LogPower > 0)
        {
            _explanation.Add($"{label} loop changes its control variable multiplicatively, so its own cost is logarithmic.");
        }
        else
        {
            _explanation.Add($"{label} loop contributes a linear factor; nested work is multiplied into it.");
        }

        return total;
    }

    private ComplexityTerm AnalyzeLocalFunction(LocalFunctionStatementSyntax localFunction, HashSet<string> visiting, int depth)
    {
        IEnumerable<StatementSyntax> statements = localFunction.Body?.Statements ?? Enumerable.Empty<StatementSyntax>();
        if (!statements.Any() && localFunction.ExpressionBody is not null)
        {
            statements = [SyntaxFactory.ReturnStatement(localFunction.ExpressionBody.Expression)];
        }

        return AnalyzeStatements(statements, visiting, depth);
    }

    private ComplexityTerm AnalyzeInvocations(SyntaxNode node, HashSet<string> visiting, int depth)
    {
        var total = ComplexityTerm.Constant;
        foreach (var invocation in node.DescendantNodesAndSelf().OfType<InvocationExpressionSyntax>())
        {
            var methodName = GetInvocationName(invocation);
            if (methodName is null || !_methods.TryGetValue(methodName, out var method))
            {
                continue;
            }

            var callComplexity = AnalyzeMethod(method, visiting, depth);
            total = ComplexityTerm.Dominant(total, callComplexity);
            _nodes.Add(new ComplexityNode($"{methodName}()", callComplexity.ToString(), callComplexity.Level, depth));
            _explanation.Add($"Method call {methodName}() was analyzed recursively and added with the sum rule.");
        }

        return total;
    }

    private static string? GetInvocationName(InvocationExpressionSyntax invocation)
    {
        return invocation.Expression switch
        {
            IdentifierNameSyntax identifier => identifier.Identifier.ValueText,
            MemberAccessExpressionSyntax memberAccess => memberAccess.Name.Identifier.ValueText,
            _ => null
        };
    }

    private static ComplexityTerm EstimateForLoop(ForStatementSyntax statement)
    {
        var incrementText = string.Join(" ", statement.Incrementors.Select(incrementor => incrementor.ToString()));
        return IsMultiplicativeUpdate(incrementText) ? ComplexityTerm.Logarithmic : ComplexityTerm.Linear;
    }

    private static ComplexityTerm EstimateWhileLoop(WhileStatementSyntax statement)
    {
        var conditionNames = statement.Condition.DescendantNodesAndSelf()
            .OfType<IdentifierNameSyntax>()
            .Select(identifier => identifier.Identifier.ValueText)
            .ToHashSet();

        var assignments = statement.Statement.DescendantNodesAndSelf()
            .Where(node => node is AssignmentExpressionSyntax or PostfixUnaryExpressionSyntax or PrefixUnaryExpressionSyntax)
            .Select(node => node.ToString());

        return assignments.Any(update => conditionNames.Any(name => update.Contains(name, StringComparison.Ordinal)) && IsMultiplicativeUpdate(update))
            ? ComplexityTerm.Logarithmic
            : ComplexityTerm.Linear;
    }

    private static ComplexityTerm EstimateDoLoop(DoStatementSyntax statement)
    {
        var loopText = statement.Statement.ToString();
        return IsMultiplicativeUpdate(loopText) ? ComplexityTerm.Logarithmic : ComplexityTerm.Linear;
    }

    private static ComplexityTerm EstimateRecursiveMethod(MethodDeclarationSyntax method)
    {
        var methodName = method.Identifier.ValueText;
        var recursiveCalls = method.DescendantNodes()
            .OfType<InvocationExpressionSyntax>()
            .Where(invocation => GetInvocationName(invocation) == methodName)
            .Select(invocation => invocation.ToString())
            .ToArray();

        if (recursiveCalls.Any(call => call.Contains("/ 2", StringComparison.Ordinal) ||
                                       call.Contains("/2", StringComparison.Ordinal) ||
                                       call.Contains(">>", StringComparison.Ordinal)))
        {
            return ComplexityTerm.Logarithmic;
        }

        return recursiveCalls.Length > 1 ? new ComplexityTerm(2, 0) : ComplexityTerm.Linear;
    }

    private static bool IsMultiplicativeUpdate(string text)
    {
        var normalized = text.Replace(" ", "", StringComparison.Ordinal);
        return normalized.Contains("*=", StringComparison.Ordinal) ||
               normalized.Contains("/=", StringComparison.Ordinal) ||
               Regex.IsMatch(normalized, @"([A-Za-z_]\w*)=\1[*/]\d+") ||
               Regex.IsMatch(normalized, @"([A-Za-z_]\w*)=\d+\*\1");
    }
}
