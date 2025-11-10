using UnityEngine;
using UnityEngine.AI;
using System.Collections;
using System.Collections.Generic;

//
// скрипт для аи врагов .-., передвижения сделаны через встроенную штуку navmesh, типо автопоиск пути
//

[RequireComponent(typeof(NavMeshAgent), typeof(MeleeAttack))]
public class EnemyAI : MonoBehaviour
{
    public static readonly List<EnemyAI> AllEnemies = new List<EnemyAI>();

    private enum AIState { Idle, Chasing, Attacking, Repositioning }
    private AIState currentState;
    private NavMeshAgent agent;
    private MeleeAttack meleeAttack;
    private Transform playerTarget;
    [SerializeField] private float detectionRadius = 15f;
    [SerializeField] private float attackRange = 2f;
    [SerializeField] private float decisionCooldown = 2.0f; // раз  сколько сек будет думать враг (expiremental functioin)
    private float lastDecisionTime;

    void OnEnable()
    {
        if (!AllEnemies.Contains(this)) AllEnemies.Add(this);
    }

    void OnDisable()
    {
        AllEnemies.Remove(this);
    }

    private void Awake()
    {
        agent = GetComponent<NavMeshAgent>();
        meleeAttack = GetComponent<MeleeAttack>();
        playerTarget = GameObject.FindGameObjectWithTag("Player")?.transform;
    }

    private void Start()
    {
        currentState = AIState.Idle;
    }

    private void Update()
    {
        if (playerTarget == null) return;

        switch (currentState)
        {
            case AIState.Idle:
                UpdateIdleState();
                break;
            case AIState.Chasing:
                UpdateChasingState();
                break;
            case AIState.Attacking:
                UpdateAttackingState();
                break;
            case AIState.Repositioning:
                UpdateRepositioningState();
                break;
        }
    }

    private void UpdateIdleState()
    {
        if (Vector3.Distance(transform.position, playerTarget.position) <= detectionRadius)
        {
            currentState = AIState.Chasing;
        }
    }

    private void UpdateChasingState()
    {
        if (Vector3.Distance(transform.position, playerTarget.position) > detectionRadius)
        {
            currentState = AIState.Idle;
            agent.isStopped = true;
            return;
        }

        if (Vector3.Distance(transform.position, playerTarget.position) <= attackRange)
        {
            DecideNextAction();
        }
        else 
        {
            agent.isStopped = false;
            agent.SetDestination(playerTarget.position);
        }
    }
    
    private void DecideNextAction()
    {
        transform.LookAt(playerTarget);

        if(Time.time - lastDecisionTime > decisionCooldown)
        {
            lastDecisionTime = Time.time;
            
            // 0.7 шанс атаковать, 0.3 шанс сменить позицию
            if (Random.Range(0f, 1f) > 0.3f) 
            {
                currentState = AIState.Attacking;
            }
            else
            {
                currentState = AIState.Repositioning;
            }
        }
    }

    private void UpdateAttackingState()
    {
        agent.isStopped = true;
        meleeAttack.PerformAttack();
        currentState = AIState.Chasing;
    }
    
    private void UpdateRepositioningState()
    {
        agent.isStopped = false;
        
        Vector3 randomDirection = Random.Range(0, 2) == 0 ? transform.right : -transform.right;
        Vector3 targetPosition = transform.position + randomDirection * Random.Range(2f, 4f);
        
        agent.SetDestination(targetPosition);

        currentState = AIState.Chasing;
    }

    private void OnDrawGizmosSelected()
    {
        Gizmos.color = Color.yellow;
        Gizmos.DrawWireSphere(transform.position, detectionRadius);
        Gizmos.color = Color.red;
        Gizmos.DrawWireSphere(transform.position, attackRange);
    }
}
