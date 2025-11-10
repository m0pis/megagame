using UnityEngine;
using System.Linq;
using System.Collections;

//
// скрипт управления персонажем, а именно прышки, роллы, бэкстепы, дэши, и тд
//

[RequireComponent(typeof(CharacterController))]
public class PlayerController : MonoBehaviour
{
    [SerializeField] private float moveSpeed = 8f;
    [SerializeField] private float rotationSpeed = 15f;
    [SerializeField] private float jumpHeight = 1.8f;
    [SerializeField] private float gravity = -25f;

    [Header("roll")]
    [SerializeField] private float rollSpeed = 16f;
    [SerializeField] private float rollDuration = 0.4f;
    [SerializeField] private float rollSteeringSpeed = 5f; // Насколько сильно можно влиять на направление в полете

    [Header("backstep")]
    [SerializeField] private float backstepSpeed = 14f;
    [SerializeField] private float backstepDuration = 0.2f; // Отскок короче и быстрее

    [Header("dash")]
    [SerializeField] private float targetDashSpeed = 22f;
    [SerializeField] private float targetDashDuration = 0.25f;
    [SerializeField] private float targetDashRandomOffset = 1.5f;

    [SerializeField] private AnimationCurve rollCurve = AnimationCurve.EaseInOut(0, 1, 1, 0);
    [SerializeField] private AnimationCurve backstepCurve = AnimationCurve.EaseInOut(0, 1, 1, 0);
    [SerializeField] private AnimationCurve targetDashCurve = AnimationCurve.EaseInOut(0, 1, 1, 0);

    [SerializeField] private KeyCode lockOnKey = KeyCode.Q;
    [SerializeField] private float lockOnRange = 20f;
    
    private CharacterController controller;
    private Transform mainCameraTransform;
    private ThirdPersonCamera thirdPersonCamera;
    
    private Vector3 playerVelocity;
    private bool isDashing = false;
    private Transform lockOnTarget;

    private void Awake()
    {
        controller = GetComponent<CharacterController>();
        mainCameraTransform = Camera.main.transform;
        thirdPersonCamera = mainCameraTransform.GetComponent<ThirdPersonCamera>();
    }

    void Update()
    {
        HandleGravity();
        HandleInput();
        
        if (isDashing) return;

        Vector2 input = new Vector2(Input.GetAxisRaw("Horizontal"), Input.GetAxisRaw("Vertical"));
        Vector3 moveDirection = GetMoveDirection(input);

        HandleMovement(moveDirection);
        HandleRotation(moveDirection);
    }

    private void HandleInput()
    {
        if (Input.GetKeyDown(lockOnKey))
        {
            ToggleLockOn();
        }
        
        if (Input.GetButtonDown("Jump") && controller.isGrounded)
        {
            Jump();
        }

        if (Input.GetKeyDown(KeyCode.LeftShift) && !isDashing)
        {
            Vector2 moveInput = new Vector2(Input.GetAxisRaw("Horizontal"), Input.GetAxisRaw("Vertical"));

            if (lockOnTarget != null && moveInput.y > 0.1f)
            {
                StartCoroutine(ExecuteDash(targetDashDuration, targetDashSpeed, targetDashCurve, GetTargetDashDirection()));
            }
            else if (moveInput.magnitude > 0.1f)
            {
                StartCoroutine(ExecuteDash(rollDuration, rollSpeed, rollCurve, GetMoveDirection(moveInput), true));
            }
            else
            {
                StartCoroutine(ExecuteDash(backstepDuration, backstepSpeed, backstepCurve, -transform.forward));
            }
        }
    }

    private IEnumerator ExecuteDash(float duration, float speed, AnimationCurve curve, Vector3 initialDirection, bool allowSteering = false)
    {
        isDashing = true;
        float startTime = Time.time;
        Vector3 dashDirection = initialDirection;

        while (Time.time < startTime + duration)
        {
            if (allowSteering)
            {
                Vector2 currentInput = new Vector2(Input.GetAxisRaw("Horizontal"), Input.GetAxisRaw("Vertical"));
                if (currentInput.magnitude > 0.1f)
                {
                    Vector3 targetDirection = GetMoveDirection(currentInput);
                    dashDirection = Vector3.Slerp(dashDirection, targetDirection, Time.deltaTime * rollSteeringSpeed);
                }
            }

            float progress = (Time.time - startTime) / duration;
            float speedMultiplier = curve.Evaluate(progress);

            controller.Move(dashDirection * speed * speedMultiplier * Time.deltaTime);
            
            yield return null;
        }
        isDashing = false;
    }

    private Vector3 GetTargetDashDirection()
    {
        Vector2 randomOffset2D = Random.insideUnitCircle * targetDashRandomOffset;
        Vector3 randomOffset = new Vector3(randomOffset2D.x, 0, randomOffset2D.y);

        Vector3 directionToTarget = (lockOnTarget.position - transform.position).normalized;

        Vector3 destination = lockOnTarget.position - directionToTarget * 1.5f + randomOffset; 

        return (destination - transform.position).normalized;
    }

    private Vector3 GetMoveDirection(Vector2 input)
    {
        return lockOnTarget != null
            ? (transform.right * input.x + transform.forward * input.y).normalized
            : (Quaternion.Euler(0, mainCameraTransform.eulerAngles.y, 0) * new Vector3(input.x, 0, input.y)).normalized;
    }

    private void HandleMovement(Vector3 moveDirection)
    {
        controller.Move(moveDirection * moveSpeed * Time.deltaTime);
    }
    
    private void HandleRotation(Vector3 moveDirection)
    {
        Vector3 lookDirection = lockOnTarget != null
            ? (lockOnTarget.position - transform.position)
            : moveDirection;
        
        lookDirection.y = 0;

        if (lookDirection.sqrMagnitude > 0.01f)
        {
            Quaternion targetRotation = Quaternion.LookRotation(lookDirection);
            transform.rotation = Quaternion.Slerp(transform.rotation, targetRotation, Time.deltaTime * rotationSpeed);
        }
    }

    private void HandleGravity()
    {
        if (controller.isGrounded && playerVelocity.y < 0)
        {
            playerVelocity.y = -2f;
        }
        playerVelocity.y += gravity * Time.deltaTime;
        controller.Move(playerVelocity * Time.deltaTime);
    }
    
    private void Jump()
    {
        playerVelocity.y = Mathf.Sqrt(jumpHeight * -2f * gravity);
    }
    
    private void ToggleLockOn()
    {
        if (lockOnTarget != null)
        {
            SetLockOnTarget(null);
        }
        else
        {
            FindAndLockTarget();
        }
    }

    private void FindAndLockTarget()
    {
        Transform closestEnemy = EnemyAI.AllEnemies
            .Where(enemy => Vector3.Distance(transform.position, enemy.transform.position) <= lockOnRange)
            .OrderBy(enemy => Vector3.Distance(transform.position, enemy.transform.position))
            .FirstOrDefault()?.transform;
        
        SetLockOnTarget(closestEnemy);
    }
    
    private void SetLockOnTarget(Transform target)
    {
        lockOnTarget = target;
        thirdPersonCamera.SetLockOnTarget(target);
    }
}
